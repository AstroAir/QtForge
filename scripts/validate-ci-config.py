#!/usr/bin/env python3
"""
QtForge CI/CD Configuration Validator

Validates GitHub Actions workflows and CI/CD configuration for best practices,
security, and performance optimization.
"""

import os
import sys
import yaml
import json
from pathlib import Path
from typing import Dict, List, Any, Optional
import re

class CIConfigValidator:
    """Validates CI/CD configuration files"""
    
    def __init__(self, repo_root: Path):
        self.repo_root = Path(repo_root)
        self.workflows_dir = self.repo_root / '.github' / 'workflows'
        self.issues = []
        self.warnings = []
        self.suggestions = []
        
    def validate_all(self) -> bool:
        """Validate all CI/CD configuration"""
        print("🔍 Validating CI/CD configuration...")
        
        success = True
        
        # Validate workflow files
        if not self._validate_workflows():
            success = False
            
        # Validate CMake configuration
        if not self._validate_cmake_config():
            success = False
            
        # Validate packaging configuration
        if not self._validate_packaging_config():
            success = False
            
        # Check for security best practices
        if not self._validate_security_practices():
            success = False
            
        # Performance optimization checks
        self._check_performance_optimizations()
        
        return success
        
    def _validate_workflows(self) -> bool:
        """Validate GitHub Actions workflow files"""
        print("  📋 Validating workflow files...")
        
        if not self.workflows_dir.exists():
            self.issues.append("No .github/workflows directory found")
            return False
            
        workflow_files = list(self.workflows_dir.glob('*.yml')) + list(self.workflows_dir.glob('*.yaml'))
        
        if not workflow_files:
            self.issues.append("No workflow files found")
            return False
            
        success = True
        for workflow_file in workflow_files:
            if not self._validate_workflow_file(workflow_file):
                success = False
                
        return success
        
    def _validate_workflow_file(self, workflow_file: Path) -> bool:
        """Validate a single workflow file"""
        try:
            with open(workflow_file, 'r') as f:
                workflow = yaml.safe_load(f)
        except yaml.YAMLError as e:
            self.issues.append(f"Invalid YAML in {workflow_file.name}: {e}")
            return False
        except Exception as e:
            self.issues.append(f"Error reading {workflow_file.name}: {e}")
            return False
            
        workflow_name = workflow_file.stem
        success = True
        
        # Check required fields
        required_fields = ['name', 'on', 'jobs']
        for field in required_fields:
            if field not in workflow:
                self.issues.append(f"{workflow_name}: Missing required field '{field}'")
                success = False
                
        # Validate jobs
        if 'jobs' in workflow:
            for job_name, job_config in workflow['jobs'].items():
                if not self._validate_job(workflow_name, job_name, job_config):
                    success = False
                    
        # Check for action versions
        self._check_action_versions(workflow_name, workflow)
        
        # Check for security issues
        self._check_workflow_security(workflow_name, workflow)
        
        return success
        
    def _validate_job(self, workflow_name: str, job_name: str, job_config: Dict[str, Any]) -> bool:
        """Validate a job configuration"""
        success = True
        
        # Check required job fields
        if 'runs-on' not in job_config:
            self.issues.append(f"{workflow_name}.{job_name}: Missing 'runs-on'")
            success = False
            
        # Check timeout
        if 'timeout-minutes' not in job_config:
            self.warnings.append(f"{workflow_name}.{job_name}: No timeout specified")
            
        # Validate steps
        if 'steps' in job_config:
            for i, step in enumerate(job_config['steps']):
                if not self._validate_step(workflow_name, job_name, i, step):
                    success = False
                    
        return success
        
    def _validate_step(self, workflow_name: str, job_name: str, step_index: int, step: Dict[str, Any]) -> bool:
        """Validate a step configuration"""
        success = True
        
        # Check for name
        if 'name' not in step:
            self.warnings.append(f"{workflow_name}.{job_name}.step[{step_index}]: No name specified")
            
        # Check for either 'uses' or 'run'
        if 'uses' not in step and 'run' not in step:
            self.issues.append(f"{workflow_name}.{job_name}.step[{step_index}]: Must have either 'uses' or 'run'")
            success = False
            
        return success
        
    def _check_action_versions(self, workflow_name: str, workflow: Dict[str, Any]) -> None:
        """Check for consistent action versions"""
        action_versions = {}
        
        def extract_actions(obj):
            if isinstance(obj, dict):
                if 'uses' in obj:
                    action = obj['uses']
                    if '@' in action:
                        name, version = action.rsplit('@', 1)
                        if name not in action_versions:
                            action_versions[name] = set()
                        action_versions[name].add(version)
                for value in obj.values():
                    extract_actions(value)
            elif isinstance(obj, list):
                for item in obj:
                    extract_actions(item)
                    
        extract_actions(workflow)
        
        # Check for inconsistent versions
        for action, versions in action_versions.items():
            if len(versions) > 1:
                self.warnings.append(f"{workflow_name}: Inconsistent versions for {action}: {', '.join(versions)}")
                
    def _check_workflow_security(self, workflow_name: str, workflow: Dict[str, Any]) -> None:
        """Check for security issues in workflow"""
        # Check for hardcoded secrets
        workflow_str = yaml.dump(workflow)
        
        # Look for potential hardcoded secrets
        secret_patterns = [
            r'password\s*[:=]\s*["\']?[^"\'\s]+["\']?',
            r'token\s*[:=]\s*["\']?[^"\'\s]+["\']?',
            r'key\s*[:=]\s*["\']?[^"\'\s]+["\']?',
        ]
        
        for pattern in secret_patterns:
            if re.search(pattern, workflow_str, re.IGNORECASE):
                # Check if it's using secrets context
                if '${{ secrets.' not in workflow_str:
                    self.warnings.append(f"{workflow_name}: Potential hardcoded secret detected")
                    
        # Check for pull_request_target usage
        if 'on' in workflow and isinstance(workflow['on'], dict):
            if 'pull_request_target' in workflow['on']:
                self.warnings.append(f"{workflow_name}: Using pull_request_target - ensure proper security")
                
    def _validate_cmake_config(self) -> bool:
        """Validate CMake configuration"""
        print("  🔧 Validating CMake configuration...")
        
        cmake_files = [
            self.repo_root / 'CMakeLists.txt',
            self.repo_root / 'CMakePresets.json'
        ]
        
        success = True
        for cmake_file in cmake_files:
            if not cmake_file.exists():
                self.warnings.append(f"Missing {cmake_file.name}")
                continue
                
            if cmake_file.suffix == '.json':
                try:
                    with open(cmake_file, 'r') as f:
                        json.load(f)
                except json.JSONDecodeError as e:
                    self.issues.append(f"Invalid JSON in {cmake_file.name}: {e}")
                    success = False
                    
        return success
        
    def _validate_packaging_config(self) -> bool:
        """Validate packaging configuration"""
        print("  📦 Validating packaging configuration...")
        
        packaging_dir = self.repo_root / 'packaging'
        if not packaging_dir.exists():
            self.warnings.append("No packaging directory found")
            return True
            
        # Check for platform-specific packaging
        expected_dirs = ['debian', 'rpm', 'macos', 'appimage', 'flatpak']
        for pkg_dir in expected_dirs:
            if not (packaging_dir / pkg_dir).exists():
                self.suggestions.append(f"Consider adding {pkg_dir} packaging support")
                
        return True
        
    def _validate_security_practices(self) -> bool:
        """Validate security best practices"""
        print("  🔒 Validating security practices...")
        
        success = True
        
        # Check for security workflow
        security_workflows = [
            'security.yml', 'security.yaml',
            'security-and-quality.yml', 'security-and-quality.yaml'
        ]
        
        has_security_workflow = any(
            (self.workflows_dir / name).exists() for name in security_workflows
        )
        
        if not has_security_workflow:
            self.suggestions.append("Consider adding a dedicated security scanning workflow")
            
        # Check for dependabot configuration
        dependabot_config = self.repo_root / '.github' / 'dependabot.yml'
        if not dependabot_config.exists():
            self.suggestions.append("Consider adding Dependabot configuration for dependency updates")
            
        return success
        
    def _check_performance_optimizations(self) -> None:
        """Check for performance optimization opportunities"""
        print("  ⚡ Checking performance optimizations...")
        
        # Check for caching in workflows
        workflow_files = list(self.workflows_dir.glob('*.yml')) + list(self.workflows_dir.glob('*.yaml'))
        
        for workflow_file in workflow_files:
            try:
                with open(workflow_file, 'r') as f:
                    content = f.read()
                    
                if 'actions/cache@' not in content:
                    self.suggestions.append(f"{workflow_file.name}: Consider adding caching for better performance")
                    
                if 'ccache' not in content:
                    self.suggestions.append(f"{workflow_file.name}: Consider using ccache for faster compilation")
                    
            except Exception:
                continue
                
    def print_report(self) -> None:
        """Print validation report"""
        print("\n" + "="*60)
        print("CI/CD CONFIGURATION VALIDATION REPORT")
        print("="*60)
        
        if self.issues:
            print(f"\n❌ ISSUES ({len(self.issues)}):")
            for issue in self.issues:
                print(f"  • {issue}")
                
        if self.warnings:
            print(f"\n⚠️  WARNINGS ({len(self.warnings)}):")
            for warning in self.warnings:
                print(f"  • {warning}")
                
        if self.suggestions:
            print(f"\n💡 SUGGESTIONS ({len(self.suggestions)}):")
            for suggestion in self.suggestions:
                print(f"  • {suggestion}")
                
        if not self.issues and not self.warnings:
            print("\n✅ Configuration looks good!")
            
        print("\n" + "="*60)

def main():
    """Main function"""
    import argparse
    
    parser = argparse.ArgumentParser(description='Validate CI/CD configuration')
    parser.add_argument('--repo-root', type=Path, default=Path('.'),
                       help='Repository root directory')
    parser.add_argument('--strict', action='store_true',
                       help='Treat warnings as errors')
    
    args = parser.parse_args()
    
    validator = CIConfigValidator(args.repo_root)
    success = validator.validate_all()
    
    validator.print_report()
    
    if not success or (args.strict and validator.warnings):
        sys.exit(1)
    else:
        print("✅ Validation completed successfully!")

if __name__ == '__main__':
    main()
