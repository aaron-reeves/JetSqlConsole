# NAADSMInst.nsi
# --------------
# Begin: 2007/06/25
# Last revision: $Date: 2007/06/28 00:34:20 $ $Author: apreeves $
# Version: $Revision: 1.2 $
# Project: JetSQLConsole
# Website: http://www.reevesdigital.com/jetsqlconsole
# Author: Aaron Reeves <development@reevesdigital.com>
# --------------------------------------------------
# Copyright (C) 2007 Aaron Reeves
# 
# This program is free software; you can redistribute it and/or modify it under the terms of the GNU General
# Public License as published by the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.


!include "AddToPath.nsh"


# Name of the Program being installed
Name "JetSQLConsole 1.0"

# NOTE: Can use the 'Icon' command to specify an icon file(.ico) for the .exe file.
Icon installerIcon.ico

# Installer Name
outFile "JetSQLConsoleSetup.exe"

# Default Installation Directory
installDir "$PROGRAMFILES\JetSQLConsole"

# Sets the text to be shown in the license page.
LicenseData ../license.txt

# Creates a custom page to ask install confirmation from the user.
page custom startDialog "" ": Start Dialog"
 Function startDialog
 
  MessageBox MB_OKCANCEL "This application will install \
  JetSQLConsole version 1.0 \ 
  on your computer. Click OK to continue." \
  IDCANCEL NoCancelAbort 
  Abort
  NoCancelAbort: Quit  
 FunctionEnd 

# Brings up the license page (pre-made in nSIS).
page license

# Asks the user to select an installation Directory (pre-made in nSIS).
page directory

# Brings up the components page(pre-made in nSIS).
#page components

# Brings up the installation dialog(pre-made in NSIS).
page instfiles


page custom endDialog "" ": End Dialog"
 Function endDialog
  MessageBox MB_OK "Installation is complete! \
  Please see http://www.reevesdigital.com/jetsqlconsole for more information. "
 FunctionEnd

# The "-" before MainSection text indicates that this section \
	cannot be selectively installed by the user, i.e., it does not \
	show up in the components page 
section "-MainSection"

# Sets the installation directory.
setOutPath $INSTDIR

# Copies each file in the installation directory.
file "C:\WINDOWS\system32\msvcrt.dll"
file "C:\MinGW\bin\mingwm10.dll"
file "C:\Qt\4.1.4\bin\QtCore4.dll"
file "..\JetSQL.exe"
file "..\cadosql.dll"
file "..\license.txt"

# Adds the install directory to the path.
Push $INSTDIR
Call AddToPath

# Creates an uninstaller.
writeUninstaller $INSTDIR\Uninstall.exe

# The following 2 commands add uninstall functionality in Add/Remove Programs:
WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\JetSQLConsole" \
                 "DisplayName" "JetSQLConsole 1.0"
WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\JetSQLConsole" \
                 "UninstallString" "$INSTDIR\Uninstall.exe"
sectionEnd

# Brings up the uninstall confirmation page before uninstalling.
uninstPage uninstConfirm "" "" ""

uninstPage instfiles

section "Uninstall"
  Delete "$INSTDIR\Uninstall.exe"
  Delete "$INSTDIR\license.txt"
  Delete "$INSTDIR\mingwm10.dll"
  Delete "$INSTDIR\msvcrt.dll"
  Delete "$INSTDIR\QtCore4.dll"
  Delete "$INSTDIR\cadosql.dll"
  Delete "$INSTDIR\JetSQL.exe"

  RMDir $INSTDIR

  Push $INSTDIR
  Call un.RemoveFromPath

  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\JetSQLConsole"

sectionEnd
