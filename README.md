# ReadMe (ja)

EasySFTPはSFTP・FTP・FTPSに対応したファイル転送を行うWindows用クライアントアプリケーションです。

## 使い方

(まだドキュメントを整理できていません。ご了承ください。)

[EasySFTP.txt](./EasySFTP.txt) をご覧ください。

## ソースコードのビルド方法

(Visual Studio 2019 における手順であり、他の環境では未確認です。)

1. OpenSSLのライブラリ(確認バージョン: 3.6.0)をソースコードで取得してビルドします (`libcrypto.lib` と `libssl.lib` のライブラリファイル、および OpenSSL のヘッダーファイルを使用します)
2. libssh2のライブラリ(確認バージョン: 1.11.1)をソースコードで取得してビルドします (`libssh2.lib` のライブラリファイル、および libssh2 のヘッダーファイルを使用します)
  - OpenSSLとlibssh2のビルド済みバイナリーは https://github.com/jet2jet/EasySFTP/actions/workflows/build-libs.yml の最新ビルドから取得できる場合があります。(保存期限が切れている場合もあります。)
3. プロジェクト `ShellDLL` に対して OpenSSL と libssh2 が利用できるように `IncludePath` と `LibraryPath` を設定します
  - `Common.user.props` ファイルを `Common.props` と同じディレクトリに置くことでこれらの設定を行うことができます。`Common.user.sample.props` ファイルは `Common.user.props` のサンプルであり、これをコピーして書き換えて利用することができます。
  - `Common.user.sample.props` には `libcrypto.lib` 、 `libssl.lib` と `libssh2.lib` を指定する設定が入っています。
4. `EasySFTP.sln` をビルドします

# ReadMe (en)

EasySFTP is a file transfer client application for Windows using SFTP, FTP, and FTPS.

Starting from v0.15.0.1, EasySFTP provides English translation.

## Build

(The following steps are for Visual Studio 2019 and not confirmed with other build systems.)

1. Download and build a OpenSSL library (checked version: 3.6.0) from source codes. (using `libcrypto.lib` and `libssl.lib` library files and OpenSSL header files)
2. Download and build a libssh2 library (checked version: 1.11.1) from source codes. (using `libssh2.lib` library files and libssh2 header files)
  - Pre-built binaries of OpenSSL and libssh2 could be download from the latest build artifacts of https://github.com/jet2jet/EasySFTP/actions/workflows/build-libs.yml (may be expired).
3. Set `IncludePath` and `LibraryPath` settings for `ShellDLL` project to use OpenSSL library.
  - To set those paths, you can create a file `Common.user.props` besides `Common.props`. The file `Common.user.sample.props` is a sample file for `Common.user.props`, and you can copy and modify this file to create `Common.user.props`.
  - The setting which uses `libcrypto.lib`, `libssl.lib`, and `libssh2.lib` is included in `Common.user.sample.props`.
4. Build `EasySFTP.sln`.

# License

[New BSD License (or The 3-Clause BSD License)](./license.txt)
