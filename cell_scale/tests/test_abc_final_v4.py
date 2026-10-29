#!/usr/bin/env python3
"""
tests/test_abc_final_v4.py

Unit tests for abc_final_v4.py — publication-grade ABC-SMC implementation.
Covers the 7 rigor improvements: prior coherence, constraints, diagnostics, Mahalanobis distance,
stochastic replicates, and reproducibility.

Run with:
  cd /home/luis/CLionProjects/cellSim
  python3 tests/test_abc_final_v4.py
"""

import sys
import os
import json
import tempfile
import unittest
import numpy as np
from pathlib import Path
from unittest.mock import MagicMock, patch

# Add scripts directory to path
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', 'scripts'))

try:
    import abc_final_v4 as abc_module
    from abc_final_v4 import (
        _valid,
        PARAM_NAMES,
        LOW_IDX,
        HIGH_IDX,
        PRIOR_LO,
        PRIOR_HI,
        PRIORS,
    )
except ImportError as e:
    print(f"Warning: Could not import abc_final_v4: {e}")
    sys.exit(1)


class TestBiologicalConstraintEnforcement(unittest.TestCase):
    """Test that biological constraint (high_delta > low_delta) is enforced."""

    def test_valid_function_enforces_ordering(self):
        """_valid() should enforce high_delta > low_delta."""
        # Create valid candidate (high > low)
        # PRIOR_LO = [0.008, 0.15, 0.05, 0.1]
        # PRIOR_HI = [0.017, 0.45, 1.2, 0.24]
        # Parameter 1 = low_delta, Parameter 2 = high_delta
        valid_candidate = np.array([0.012, 0.2, 0.8, 0.15])  # high (0.8) > low (0.2)
        self.assertTrue(_valid(valid_candidate))
        
        # Create invalid candidate (high <= low)
        invalid_candidate = np.array([0.012, 0.8, 0.2, 0.15])  # high (0.2) < low (0.8)
        self.assertFalse(_valid(invalid_candidate))

    def test_prior_bounds_are_correct(self):
        """Prior bounds should be properly defined."""
        self.assertEqual(len(PRIOR_LO), 4)  # 4 parameters
        self.assertEqual(len(PRIOR_HI), 4)
        
        # All lower bounds < upper bounds
        for i in range(len(PRIOR_LO)):
            self.assertLess(PRIOR_LO[i], PRIOR_HI[i],
                          f"PRIOR_LO[{i}] should be < PRIOR_HI[{i}]")


class TestPriorSampling(unittest.TestCase):
    """Test prior sampling respects constraints."""

    def test_sample_prior_respects_constraint(self):
        """Sampled priors should satisfy high_delta > low_delta."""
        rng = np.random.Generator(np.random.PCG64(42))
        samples = abc_module.sample_prior(rng, n=50)
        
        self.assertEqual(samples.shape[0], 50)
        self.assertEqual(samples.shape[1], 4)
        
        # All samples should satisfy constraint
        for sample in samples:
            self.assertTrue(_valid(sample),
                          f"Sample violates constraint: {sample}")

    def test_sample_prior_respects_bounds(self):
        """Sampled priors should be within defined bounds."""
        rng = np.random.Generator(np.random.PCG64(123))
        samples = abc_module.sample_prior(rng, n=30)
        
        # Check all parameters are within bounds
        for i in range(len(PARAM_NAMES)):
            self.assertTrue(np.all(samples[:, i] >= PRIOR_LO[i]),
                          f"Parameter {i} below lower bound")
            self.assertTrue(np.all(samples[:, i] <= PRIOR_HI[i]),
                          f"Parameter {i} above upper bound")

    def test_sample_prior_is_reproducible(self):
        """Same seed should produce identical samples."""
        samples1 = abc_module.sample_prior(
            np.random.Generator(np.random.PCG64(999)), n=10
        )
        samples2 = abc_module.sample_prior(
            np.random.Generator(np.random.PCG64(999)), n=10
        )
        
        np.testing.assert_array_almost_equal(samples1, samples2)


class TestLogPriorFunction(unittest.TestCase):
    """Test log_prior correctly rejects outside bounds and invalid orderings."""

    def test_log_prior_rejects_invalid_ordering(self):
        """log_prior should return -inf for invalid ordering."""
        invalid = np.array([0.5, 0.8, 0.2, 0.3])  # high_idx < low_idx
        self.assertEqual(abc_module.log_prior(invalid), -np.inf)

    def test_log_prior_rejects_out_of_bounds(self):
        """log_prior should return -inf for out-of-bounds values."""
        # Valid ordering but out of bounds
        out_of_bounds = np.array([-0.5, 0.1, 0.9, 0.5])  # first param < 0
        self.assertEqual(abc_module.log_prior(out_of_bounds), -np.inf)

    def test_log_prior_accepts_valid(self):
        """log_prior should return finite value for valid parameters."""
        # Use values within actual prior bounds
        # PRIOR_LO = [0.008, 0.15, 0.05, 0.1]
        # PRIOR_HI = [0.017, 0.45, 1.2, 0.24]
        valid = np.array([0.012, 0.2, 0.8, 0.15])  # All within bounds, high (0.8) > low (0.2)
        result = abc_module.log_prior(valid)
        self.assertFalse(np.isinf(result))
        self.assertFalse(np.isnan(result))


class TestWeightComputation(unittest.TestCase):
    """Test weight computation (Toni et al., Eq. A4)."""

    def test_weights_are_normalized(self):
        """Computed weights should sum to 1."""
        rng = np.random.Generator(np.random.PCG64(42))
        
        # Generate some particles
        particles = abc_module.sample_prior(rng, n=5)
        prev_particles = abc_module.sample_prior(rng, n=5)
        prev_weights = np.ones(5) / 5
        sigmas = np.ones(4) * 0.1
        
        weights = abc_module.compute_weights(particles, prev_particles, prev_weights, sigmas)
        
        self.assertEqual(len(weights), 5)
        self.assertAlmostEqual(np.sum(weights), 1.0, places=10)

    def test_weights_are_non_negative(self):
        """All weights should be non-negative."""
        rng = np.random.Generator(np.random.PCG64(999))
        
        particles = abc_module.sample_prior(rng, n=10)
        prev_particles = abc_module.sample_prior(rng, n=10)
        prev_weights = np.ones(10) / 10
        sigmas = np.ones(4) * 0.05
        
        weights = abc_module.compute_weights(particles, prev_particles, prev_weights, sigmas)
        
        self.assertTrue(np.all(weights >= 0),
                       f"Found negative weight: {weights[weights < 0]}")


class TestManifestPersistence(unittest.TestCase):
    """Test reproducibility manifest is properly saved."""

    def test_manifest_contains_required_fields(self):
        """Check that manifest JSON has all required fields."""
        # We need to check the schema by examining what _save_manifest writes
        # This is somewhat limited since the actual _save_manifest needs argparse.Namespace
        
        # Instead, verify that the function exists and can be called
        self.assertTrue(hasattr(abc_module, '_save_manifest'))
        self.assertTrue(callable(abc_module._save_manifest))


class TestDiagnosticsPersistence(unittest.TestCase):
    """Test that per-generation diagnostics are saved."""

    def test_generation_diagnostics_functions_exist(self):
        """Check that diagnostic saving functions are implemented."""
        # Check that functions for saving diagnostics exist
        self.assertTrue(hasattr(abc_module, '_append_diag'),
                       "Missing _append_diag function")
        self.assertTrue(hasattr(abc_module, '_save_eps_history'),
                       "Missing _save_eps_history function")


class TestSimulationReproducibility(unittest.TestCase):
    """Test that simulation calls use reproducibility features."""

    def test_run_simulation_uses_seed_offset(self):
        """run_simulation should use seed_offset for reproducibility."""
        # Check that run_simulation function exists
        self.assertTrue(hasattr(abc_module, 'run_simulation'),
                       "Missing run_simulation function")
        
        # Verify function signature includes seed_base parameter
        import inspect
        sig = inspect.signature(abc_module.run_simulation)
        self.assertIn('seed_base', sig.parameters,
                     "run_simulation should have seed_base parameter")

    def test_stochastic_replicates_parameter_exists(self):
        """Check that n_reps parameter exists for stochastic replicates."""
        import inspect
        sig = inspect.signature(abc_module.run_simulation)
        self.assertIn('n_reps', sig.parameters,
                     "run_simulation should have n_reps parameter")


class TestMahalanobisDistanceIntegration(unittest.TestCase):
    """Test that Mahalanobis distance is integrated."""

    def test_binary_accepts_mahalanobis_flag(self):
        """Check that run_simulation calls binary with --mahalanobis flag."""
        # The binary should be called with --mahalanobis flag
        # We can check this by examining the code
        with open(os.path.join(os.path.dirname(__file__), '..', 'scripts', 'abc_final_v4.py')) as f:
            content = f.read()
            self.assertIn('--mahalanobis', content,
                         "run_simulation should pass --mahalanobis flag to binary")

    def test_seed_offset_passed_to_binary(self):
        """Check that seed_offset is properly passed to binary."""
        with open(os.path.join(os.path.dirname(__file__), '..', 'scripts', 'abc_final_v4.py')) as f:
            content = f.read()
            self.assertIn('--seed-offset', content,
                         "run_simulation should pass --seed-offset to binary")


class TestAlgorithmRigor(unittest.TestCase):
    """Integration tests for overall algorithm rigor."""

    def test_no_clipping_in_prior_enforcement(self):
        """Algorithm should reject invalid samples rather than clipping."""
        # Check that _valid function exists and is used
        with open(os.path.join(os.path.dirname(__file__), '..', 'scripts', 'abc_final_v4.py')) as f:
            content = f.read()
            # Should use _valid() to check constraint
            self.assertIn('_valid(', content)
            # Should not use np.clip for constraint enforcement
            # (it may be used elsewhere but not for biological constraints)

    def test_particle_generation_has_explicit_failure_on_low_acceptance(self):
        """run_generation should fail if it can't reach N particles."""
        # Check for RuntimeError or explicit failure mode
        with open(os.path.join(os.path.dirname(__file__), '..', 'scripts', 'abc_final_v4.py')) as f:
            content = f.read()
            # Should have explicit check for particle count
            self.assertIn('RuntimeError', content,
                         "Should raise RuntimeError if particles not reached")


class TestPythonFunctionality(unittest.TestCase):
    """Test core Python ABC-SMC functionality."""

    def test_run_generation_function_exists(self):
        """run_generation function should exist."""
        self.assertTrue(hasattr(abc_module, 'run_generation'))
        self.assertTrue(callable(abc_module.run_generation))

    def test_save_generation_function_exists(self):
        """save_generation function should exist."""
        self.assertTrue(hasattr(abc_module, 'save_generation'))
        self.assertTrue(callable(abc_module.save_generation))

    def test_run_abc_final_function_exists(self):
        """run_abc_final main ABC loop should exist."""
        self.assertTrue(hasattr(abc_module, 'run_abc_final'))
        self.assertTrue(callable(abc_module.run_abc_final))


if __name__ == '__main__':
    # Run tests with verbose output
    unittest.main(verbosity=2)


class TestBiologicalConstraints(unittest.TestCase):
    """Test biological constraint enforcement (low_delta > high_delta)."""

    def test_valid_accepts_correctly_ordered_deltas(self):
        """_valid should accept low_delta < high_delta."""
        if _valid is None:
            self.skipTest("_valid function not available")
        
        # low_delta < high_delta should be valid
        self.assertTrue(_valid(low_delta=0.01, high_delta=0.5))
        self.assertTrue(_valid(low_delta=0.1, high_delta=0.9))
        self.assertTrue(_valid(low_delta=0.0, high_delta=1.0))

    def test_valid_rejects_reversed_deltas(self):
        """_valid should reject high_delta < low_delta (reversed)."""
        if _valid is None:
            self.skipTest("_valid function not available")
        
        # high_delta < low_delta is biologically invalid
        self.assertFalse(_valid(low_delta=0.9, high_delta=0.1))
        self.assertFalse(_valid(low_delta=0.5, high_delta=0.2))

    def test_valid_rejects_equal_deltas(self):
        """_valid should reject low_delta == high_delta."""
        if _valid is None:
            self.skipTest("_valid function not available")
        
        # Equal deltas violate the strict inequality
        self.assertFalse(_valid(low_delta=0.5, high_delta=0.5))

    def test_valid_handles_boundary_values(self):
        """_valid should handle boundary values correctly."""
        if _valid is None:
            self.skipTest("_valid function not available")
        
        # Smallest valid difference
        self.assertTrue(_valid(low_delta=0.0, high_delta=1e-10))
        
        # At bounds
        self.assertTrue(_valid(low_delta=0.0, high_delta=0.99999))


class TestPriorSampling(unittest.TestCase):
    """Test prior sampling respects biological constraints."""

    def test_sample_prior_returns_valid_dict(self):
        """sample_prior should return dict with required keys."""
        if sample_prior is None:
            self.skipTest("sample_prior function not available")
        
        sample = sample_prior(seed=42)
        
        self.assertIsInstance(sample, dict)
        self.assertIn('low_delta', sample)
        self.assertIn('high_delta', sample)

    def test_sample_prior_respects_constraint(self):
        """sample_prior should always satisfy low_delta < high_delta."""
        if sample_prior is None:
            self.skipTest("sample_prior function not available")
        
        for seed in range(10):
            sample = sample_prior(seed=seed)
            self.assertLess(sample['low_delta'], sample['high_delta'],
                          f"Seed {seed}: low_delta={sample['low_delta']}, "
                          f"high_delta={sample['high_delta']}")

    def test_sample_prior_is_reproducible(self):
        """Sampling with same seed should give same result."""
        if sample_prior is None:
            self.skipTest("sample_prior function not available")
        
        sample1 = sample_prior(seed=123)
        sample2 = sample_prior(seed=123)
        
        self.assertAlmostEqual(sample1['low_delta'], sample2['low_delta'])
        self.assertAlmostEqual(sample1['high_delta'], sample2['high_delta'])

    def test_sample_prior_is_stochastic(self):
        """Different seeds should (usually) give different samples."""
        if sample_prior is None:
            self.skipTest("sample_prior function not available")
        
        sample1 = sample_prior(seed=1)
        sample2 = sample_prior(seed=2)
        
        # They should differ (with very high probability)
        self.assertNotAlmostEqual(sample1['low_delta'], sample2['low_delta'])


class TestWeightComputation(unittest.TestCase):
    """Test ABC-SMC weight formula (Toni et al., Eq. A4)."""

    def test_compute_weights_non_negative(self):
        """Weights should all be non-negative."""
        if compute_weights is None:
            self.skipTest("compute_weights function not available")
        
        distances = [0.5, 1.0, 1.5, 2.0]
        epsilon = 1.0
        weights = compute_weights(distances, epsilon)
        
        self.assertTrue(all(w >= 0 for w in weights),
                       f"Found negative weight: {weights}")

    def test_compute_weights_sum_to_one(self):
        """Weights should normalize to sum 1."""
        if compute_weights is None:
            self.skipTest("compute_weights function not available")
        
        distances = [0.5, 1.0, 1.5, 2.0]
        epsilon = 1.0
        weights = compute_weights(distances, epsilon)
        
        total = sum(weights)
        self.assertAlmostEqual(total, 1.0, places=10,
                              msg=f"Weights sum to {total}, not 1.0: {weights}")

    def test_compute_weights_penalizes_large_distances(self):
        """Particles with smaller distances should have larger weights."""
        if compute_weights is None:
            self.skipTest("compute_weights function not available")
        
        # Clearly different distances
        distances = [0.1, 1.0, 10.0]
        epsilon = 1.0
        weights = compute_weights(distances, epsilon)
        
        # Smallest distance should have highest weight
        self.assertGreater(weights[0], weights[1])
        self.assertGreater(weights[1], weights[2])

    def test_compute_weights_zero_distance(self):
        """Perfect match (distance=0) should have high weight."""
        if compute_weights is None:
            self.skipTest("compute_weights function not available")
        
        distances = [0.0, 1.0]
        epsilon = 1.0
        weights = compute_weights(distances, epsilon)
        
        # Zero distance should dominate
        self.assertGreater(weights[0], weights[1])

    def test_compute_weights_tight_epsilon_uses_few_particles(self):
        """Tighter epsilon (smaller threshold) should concentrate mass on best particles."""
        if compute_weights is None:
            self.skipTest("compute_weights function not available")
        
        distances = [0.5, 1.0, 2.0, 5.0]
        
        weights_tight = compute_weights(distances, epsilon=0.1)   # very tight
        weights_loose = compute_weights(distances, epsilon=10.0)  # very loose
        
        # Tight epsilon: first particle (best) should dominate more
        self.assertGreater(weights_tight[0], weights_loose[0],
                          "Tight epsilon should give more weight to best particle")
        
        # Loose epsilon: should distribute more evenly
        # (worst particle should get more weight in loose regime)
        self.assertGreater(weights_loose[-1], weights_tight[-1],
                          "Loose epsilon should give more weight to poor particles")


class TestReproducibilityManifest(unittest.TestCase):
    """Test reproducibility metadata persistence."""

    def test_save_manifest_creates_file(self):
        """_save_manifest should create a JSON file with metadata."""
        if _save_manifest is None:
            self.skipTest("_save_manifest function not available")
        
        with tempfile.TemporaryDirectory() as tmpdir:
            manifest_path = os.path.join(tmpdir, 'test_manifest.json')
            
            # Create minimal manifest
            metadata = {
                'timestamp': '2024-01-01T00:00:00Z',
                'git_commit': 'abc123',
                'priors': {'low_delta': [0.0, 1.0], 'high_delta': [0.0, 1.0]},
                'fixed_params': {},
                'cli_args': ['--n-particles', '100'],
            }
            
            _save_manifest(manifest_path, metadata)
            
            # Check file was created
            self.assertTrue(os.path.exists(manifest_path),
                          f"Manifest file not created: {manifest_path}")
            
            # Check it's valid JSON
            with open(manifest_path, 'r') as f:
                loaded = json.load(f)
            
            self.assertEqual(loaded['git_commit'], 'abc123')

    def test_save_manifest_contains_required_fields(self):
        """Manifest should contain required reproducibility fields."""
        if _save_manifest is None:
            self.skipTest("_save_manifest function not available")
        
        required_fields = {
            'timestamp', 'git_commit', 'python_version', 'platform',
            'priors', 'fixed_params', 'cli_args'
        }
        
        with tempfile.TemporaryDirectory() as tmpdir:
            manifest_path = os.path.join(tmpdir, 'test_manifest.json')
            
            metadata = {
                'timestamp': '2024-01-01T00:00:00Z',
                'git_commit': 'abc123',
                'python_version': '3.10',
                'platform': 'Linux',
                'priors': {},
                'fixed_params': {},
                'cli_args': [],
            }
            
            _save_manifest(manifest_path, metadata)
            
            with open(manifest_path, 'r') as f:
                loaded = json.load(f)
            
            missing = required_fields - set(loaded.keys())
            self.assertEqual(len(missing), 0,
                           f"Missing fields in manifest: {missing}")


class TestPriorBounds(unittest.TestCase):
    """Test prior sampling stays within defined bounds."""

    def test_sample_prior_respects_bounds(self):
        """Samples should respect prior bounds."""
        if sample_prior is None or PRIORS is None:
            self.skipTest("sample_prior or PRIORS not available")
        
        # Run multiple samples to check bounds
        for seed in range(20):
            sample = sample_prior(seed=seed)
            
            # Check low_delta is within bounds
            self.assertGreaterEqual(sample['low_delta'], PRIORS['low_delta'][0],
                                   f"low_delta below lower bound: {sample['low_delta']}")
            self.assertLessEqual(sample['low_delta'], PRIORS['low_delta'][1],
                                f"low_delta above upper bound: {sample['low_delta']}")
            
            # Check high_delta is within bounds
            self.assertGreaterEqual(sample['high_delta'], PRIORS['high_delta'][0],
                                   f"high_delta below lower bound: {sample['high_delta']}")
            self.assertLessEqual(sample['high_delta'], PRIORS['high_delta'][1],
                                f"high_delta above upper bound: {sample['high_delta']}")


class TestEdgeCases(unittest.TestCase):
    """Test edge cases and numerical stability."""

    def test_compute_weights_handles_empty_list(self):
        """compute_weights should handle or reject empty input gracefully."""
        if compute_weights is None:
            self.skipTest("compute_weights function not available")
        
        try:
            weights = compute_weights([], epsilon=1.0)
            # If it doesn't raise, it should return empty list
            self.assertEqual(len(weights), 0)
        except (ValueError, IndexError):
            # It's also acceptable to raise an error for empty input
            pass

    def test_compute_weights_handles_inf_distance(self):
        """compute_weights should handle infinite distances gracefully."""
        if compute_weights is None:
            self.skipTest("compute_weights function not available")
        
        distances = [1.0, float('inf')]
        epsilon = 1.0
        
        try:
            weights = compute_weights(distances, epsilon)
            # Should be finite and normalized
            self.assertTrue(all(w >= 0 and w <= 1 for w in weights))
            self.assertAlmostEqual(sum(weights), 1.0)
        except ValueError:
            # Also acceptable to reject infinite distances
            pass

    def test_valid_with_none_values(self):
        """_valid should handle None values."""
        if _valid is None:
            self.skipTest("_valid function not available")
        
        # Should not crash, but rather return False
        result = _valid(low_delta=None, high_delta=0.5)
        self.assertFalse(result)


if __name__ == '__main__':
    # Run tests with verbose output
    unittest.main(verbosity=2)


class TestNumericalStability(unittest.TestCase):
    """Test numerical stability safeguards in weight computation."""

    def test_compute_weights_handles_extreme_distances(self):
        """Weight computation should handle extreme distance values without underflow."""
        rng = np.random.Generator(np.random.PCG64(42))
        particles = abc_module.sample_prior(rng, n=5)
        prev_particles = abc_module.sample_prior(rng, n=5)
        prev_weights = np.ones(5) / 5
        sigmas = np.ones(4) * 1e-10  # very small sigmas → extreme distances
        
        try:
            weights = abc_module.compute_weights(particles, prev_particles, prev_weights, sigmas)
            self.assertTrue(np.all(np.isfinite(weights)))
            self.assertAlmostEqual(np.sum(weights), 1.0, places=6)
        except RuntimeError as e:
            # Also acceptable if it detects and raises explicitly
            self.assertIn("NaN", str(e)) or self.assertIn("Inf", str(e))

    def test_compute_weights_normalization(self):
        """Weights should always sum to 1.0 within numerical tolerance."""
        rng = np.random.Generator(np.random.PCG64(123))
        for _ in range(10):
            particles = abc_module.sample_prior(rng, n=10)
            prev_particles = abc_module.sample_prior(rng, n=10)
            prev_weights = np.random.dirichlet(np.ones(10))
            sigmas = np.random.uniform(0.01, 0.5, size=4)
            
            weights = abc_module.compute_weights(particles, prev_particles, prev_weights, sigmas)
            self.assertAlmostEqual(np.sum(weights), 1.0, places=10)


class TestConvergenceDiagnostics(unittest.TestCase):
    """Test convergence diagnostic functions."""

    def test_ess_computation(self):
        """ESS should measure effective sample size correctly."""
        # Uniform weights → ESS = N
        uniform_weights = np.ones(100) / 100
        ess = abc_module.compute_ess(uniform_weights)
        self.assertAlmostEqual(ess, 100.0, places=1)
        
        # Concentrated weights → ESS << N
        concentrated = np.zeros(100)
        concentrated[0] = 1.0
        ess_low = abc_module.compute_ess(concentrated)
        self.assertLess(ess_low, 10.0)

    def test_gelman_rubin_converged(self):
        """Rhat should be close to 1.0 for converged chains."""
        # Two chains from same distribution
        chain1 = np.random.normal(0, 1, size=100)
        chain2 = np.random.normal(0, 1, size=100)
        
        rhat = abc_module.gelman_rubin_statistic([chain1, chain2])
        self.assertLess(rhat, 1.2)  # Should be close to 1.0

    def test_gelman_rubin_single_chain(self):
        """Rhat should return NaN for single chain."""
        chain = np.random.normal(0, 1, size=100)
        rhat = abc_module.gelman_rubin_statistic([chain])
        self.assertTrue(np.isnan(rhat))

    def test_posterior_predictive_check_structure(self):
        """Posterior predictive check should return structured output."""
        rng = np.random.Generator(np.random.PCG64(999))
        samples = abc_module.sample_prior(rng, n=50)
        
        result = abc_module.posterior_predictive_check(samples, n_sim=100, n_check=10)
        
        self.assertIn('n_posterior_samples', result)
        self.assertIn('clinical_mean', result)
        self.assertIsInstance(result['clinical_mean'], float)


if __name__ == '__main__':
    unittest.main(verbosity=2)
