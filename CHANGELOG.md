# ChangeLog

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
