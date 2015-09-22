"""
A Pygments lexer for Wren.
"""
from setuptools import setup

__author__ = 'Robert Nystrom'

setup(
    name='Wren',
    version='1.0',
    description=__doc__,
    author=__author__,
    packages=['wren'],
    entry_points='''
    [pygments.lexers]
    wrenlexer = wren:WrenLexer
    '''
)