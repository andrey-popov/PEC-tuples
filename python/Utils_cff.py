"""Utility classes and functions used in the main configuration."""

import FWCore.ParameterSet.Config as cms


class PathManager:
    """A class to work with multiple CMS paths simultaneuosly."""
    
    def __init__(self, *paths):
        """Construct from an arbitrary number of cms.Path."""
        
        self.paths = list(paths)
    
    
    def append(self, *modules):
        """Append one or more modules to each path."""
        
        for p in self.paths:
            for m in modules:
                p += m

