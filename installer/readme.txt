A. Reeves, June 25, 2007 
<development@reevesdigital.com>
--------------------------------

Installer script for JetSQLConsole
==================================

This installer script requires the Nullsoft Scriptable Install Ststem
(http://nsis.sourceforge.net).  It also requires functionality for the
modification of the PATH environment variable: functions for this capability
may be found at http://nsis.sourceforge.net/Path_Manipulation, and are included
in the source file AddToPath.nsh in this directory.

FIX ME: Are these warnings normal?
2 warnings:
  uninstall function "un.RemoveFromEnvVar" not referenced - zeroing code (66-140) out

  install function "AddToEnvVar" not referenced - zeroing code (55-104) out

