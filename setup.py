"""Installation script for games."""
from setuptools import setup, find_packages, Extension
binary_puzzle = Extension(
    'binary_puzzle',
    sources=['games/binarypuzzlemodule.c'])

setup(
    name='games',
    version='1.0',
    description='online character sheets for Dungeons & Dragons v2.thomaas',
    author='nihlaeth',
    author_email='info@nihlaeth.nl',
    python_requires='>=3.6',
    packages=find_packages(),
    ext_modules=[binary_puzzle],
    install_requires=[
        'python-constraint',
        'user_config',
        'aiohttp',
        'aiosmtplib>=1.0.1',
        'markupsafe',
        'markdown',
        'aiohttp_session[secure]',
        'Jinja2',
        'aiohttp_jinja2',
        'motor',
        'cchardet',
        'aiodns',
        'aiohttp-login',
        'pyyaml',
        'uvloop',
        'pyhtml>=1.1.2'],
    entry_points={
        'console_scripts': ['generate_binary_puzzle = games.binary_puzzle_tools:start']},
    package_data={'games': ['static/*', 'templates/*', 'config/*']},
    )
