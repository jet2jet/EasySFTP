# ReadMe (ja)

EasySFTPはFTPとSFTPに対応したファイル転送を行うWindows用クライアントアプリケーションです。

現在は更新・メンテナンスを停止しています。

## ビルド方法

(Visual Studio 2019 における手順であり、他の環境では未確認です。)

1. OpenSSLのライブラリ(推奨バージョン: 1.1.1k)をソースコードで取得してビルドします (`libcrypto.lib` と `libssl.lib` のライブラリファイル、および OpenSSL のヘッダーファイルを使用します)
2. プロジェクト `ShellDLL` に対して OpenSSL が利用できるように `IncludePath` と `LibraryPath` を設定します
  * `Common.user.props` ファイルを `Common.props` と同じディレクトリに置くことでこれらの設定を行うことができます。`Common.user.sample.props` ファイルは `Common.user.props` のサンプルであり、これをコピーして書き換えて利用することができます。
  * `Common.user.sample.props` には `libcrypto.lib` と `libssl.lib` を指定する設定が入っています。
3. `EasySFTP.sln` をビルドします

# ReadMe (en)

EasySFTP is a file transfer client application for Windows using FTP and SFTP.

Currently no update and maintenance is planned for this application.

## Build

(The following steps are for Visual Studio 2019 and not confirmed with other build systems.)

1. Download and build a OpenSSL library (recommended version: 1.1.1k) from source codes. (using `libcrypto.lib` and `libssl.lib` library files and OpenSSL header files)
2. Set `IncludePath` and `LibraryPath` settings for `ShellDLL` project to use OpenSSL library.
  * To set those paths, you can create a file `Common.user.props` besides `Common.props`. The file `Common.user.sample.props` is a sample file for `Common.user.props`, and you can copy and modify this file to create `Common.user.props`.
  * The setting which uses `libcrypto.lib` and `libssl.lib` is included in `Common.user.sample.props`.
3. Build `EasySFTP.sln`.

# License

[New BSD License (or The 3-Clause BSD License)](./license.txt)
