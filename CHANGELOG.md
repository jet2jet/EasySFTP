# ChangeLog

## v0.13.0.1

- Support FTPS (FTP over SSL/TLS)
  - EasySFTP uses Explicit mode
	- Data transfer is also encrypted (using `PROT P` command)
  - `ftps` protocol (schema) is used
- Make new connection when establishing data connection for FTP/FTPS
  - This enables multiple transaction
- Add 'Synchronize' feature
  - This works on both remote and local directories (e.g. local <-> remote, local <-> local)
- Fix some bugs

## v0.12.0.1

- Add implementation for use from VBA, PowerShell, and etc.
  - EasySFTP.dll now provides type library
- Restore showing icons on transfer dialog
- Fix some bugs, especially for Shell Extension

## v0.11.1.1

- Fix for reauthenticating process on SFTP
- Add registration feature for per-user mode (register to HKEY_CURRENT_USER, not HKEY_CLASSES_ROOT)
- Fix for using Windows OpenSSH ssh-agent from Shell Extension
- Fix some bugs for Shell Extension, including
  fix for unable to launch app when registered
- Fix registry emulation (after fix, this works on Windows 7)

## v0.11.0.1

- Add features for directory operations (upload/download directories and 'Upload all' 'Download all')
- Enable 'cut' operation (`Ctrl+X`)
- Fix to work sorting with File Type properly
- Remove `.` directory from the list in FTP mode
- Fix keep-alive requests (fix for not working timer)
- Add keep-alive calls for SFTP
- Add implementations for some extra Shell interfaces

## v0.10.3.1

- Fix for app crash on referencing child items on EasySFTP root

## v0.10.2.1

- Fix for empty string usage
- Fix for specifying item-id-list
- Add link to repository on about box

## v0.10.1.1

- Fix for sending large data on SFTP

## v0.10.0.1

- Replace SSH implementation with [libssh2](https://www.libssh2.org/) (drop SSH1 support)
- Remove dependency for PuTTY (Pageant is still supported)
- Support ssh-agent of Windows OpenSSH (which bundled in Windows 10)
- Fix drag and drop / copy-and-paste operation
- Fix some minor bugs

## v0.9.3.3

(This is the first version which is committed and pushed to GitHub)
