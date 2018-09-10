# ReadMe (ja)

EasySFTPはFTPとSFTPに対応したファイル転送を行うWindows用クライアントアプリケーションです。

現在は更新・メンテナンスを停止しています。

## ビルド方法

(Visual Studio 2015 における手順であり、他の環境では未確認です。)

1. OpenSSLのライブラリをソースコードで取得してビルドします (`libeay32.lib` と `ssleay32.lib` のライブラリファイル、および OpenSSL のヘッダーファイルを使用します)
2. プロジェクト `ShellDLL` に対して OpenSSL が利用できるように `IncludePath` と `LibraryPath` を設定します
  * `Common.user.props` ファイルを `Common.props` と同じディレクトリに置くことでこれらの設定を行うことができます。`Common.user.sample.props` ファイルは `Common.user.props` のサンプルであり、これをコピーして書き換えて利用することができます。
3. `EasySFTP.vs14.sln` をビルドします

# ReadMe (en)

EasySFTP is a file transfer client application for Windows using FTP and SFTP.

Currently no update and maintenance is planned for this application.

## Build

(The following steps are for Visual Studio 2015 and not confirmed with other build systems.)

1. Download and build a OpenSSL library from source codes. (using `libeay32.lib` and `ssleay32.lib` library files and OpenSSL header files)
2. Set `IncludePath` and `LibraryPath` settings for `ShellDLL` project to use OpenSSL library.
  * To set those paths, you can create a file `Common.user.props` besides `Common.props`. The file `Common.user.sample.props` is a sample file for `Common.user.props`, and you can copy and modify this file to create `Common.user.props`.
3. Build `EasySFTP.vs14.sln`.

# License

[New BSD License (or The 3-Clause BSD License)](./license.txt)
