

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 8.01.0628 */
/* at Tue Jan 19 12:14:07 2038
 */
/* Compiler settings for ShellDLL.idl:
    Oicf, W1, Zp8, env=Win64 (32b run), target_arch=AMD64 8.01.0628 
    protocol : all , ms_ext, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
/* @@MIDL_FILE_HEADING(  ) */



/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 500
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif /* __RPCNDR_H_VERSION__ */


#ifndef __EasySFTP_h_h__
#define __EasySFTP_h_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#ifndef DECLSPEC_XFGVIRT
#if defined(_CONTROL_FLOW_GUARD_XFG)
#define DECLSPEC_XFGVIRT(base, func) __declspec(xfg_virtual(base, func))
#else
#define DECLSPEC_XFGVIRT(base, func)
#endif
#endif

/* Forward Declarations */ 

#ifndef __IEasySFTPFile_FWD_DEFINED__
#define __IEasySFTPFile_FWD_DEFINED__
typedef interface IEasySFTPFile IEasySFTPFile;

#endif 	/* __IEasySFTPFile_FWD_DEFINED__ */


#ifndef __IEasySFTPFiles_FWD_DEFINED__
#define __IEasySFTPFiles_FWD_DEFINED__
typedef interface IEasySFTPFiles IEasySFTPFiles;

#endif 	/* __IEasySFTPFiles_FWD_DEFINED__ */


#ifndef __IEasySFTPStream_FWD_DEFINED__
#define __IEasySFTPStream_FWD_DEFINED__
typedef interface IEasySFTPStream IEasySFTPStream;

#endif 	/* __IEasySFTPStream_FWD_DEFINED__ */


#ifndef __IEasySFTPDirectory_FWD_DEFINED__
#define __IEasySFTPDirectory_FWD_DEFINED__
typedef interface IEasySFTPDirectory IEasySFTPDirectory;

#endif 	/* __IEasySFTPDirectory_FWD_DEFINED__ */


#ifndef __IEasySFTPRootDirectory_FWD_DEFINED__
#define __IEasySFTPRootDirectory_FWD_DEFINED__
typedef interface IEasySFTPRootDirectory IEasySFTPRootDirectory;

#endif 	/* __IEasySFTPRootDirectory_FWD_DEFINED__ */


#ifndef __IEasySFTPAuthentication_FWD_DEFINED__
#define __IEasySFTPAuthentication_FWD_DEFINED__
typedef interface IEasySFTPAuthentication IEasySFTPAuthentication;

#endif 	/* __IEasySFTPAuthentication_FWD_DEFINED__ */


#ifndef __IEasySFTPRoot_FWD_DEFINED__
#define __IEasySFTPRoot_FWD_DEFINED__
typedef interface IEasySFTPRoot IEasySFTPRoot;

#endif 	/* __IEasySFTPRoot_FWD_DEFINED__ */


#ifndef __IEasySFTPStringList_FWD_DEFINED__
#define __IEasySFTPStringList_FWD_DEFINED__
typedef interface IEasySFTPStringList IEasySFTPStringList;

#endif 	/* __IEasySFTPStringList_FWD_DEFINED__ */


#ifndef __IEasySFTPHostSetting_FWD_DEFINED__
#define __IEasySFTPHostSetting_FWD_DEFINED__
typedef interface IEasySFTPHostSetting IEasySFTPHostSetting;

#endif 	/* __IEasySFTPHostSetting_FWD_DEFINED__ */


#ifndef __IEasySFTPHostSettingList_FWD_DEFINED__
#define __IEasySFTPHostSettingList_FWD_DEFINED__
typedef interface IEasySFTPHostSettingList IEasySFTPHostSettingList;

#endif 	/* __IEasySFTPHostSettingList_FWD_DEFINED__ */


#ifndef __EasySFTPRoot_FWD_DEFINED__
#define __EasySFTPRoot_FWD_DEFINED__

#ifdef __cplusplus
typedef class EasySFTPRoot EasySFTPRoot;
#else
typedef struct EasySFTPRoot EasySFTPRoot;
#endif /* __cplusplus */

#endif 	/* __EasySFTPRoot_FWD_DEFINED__ */


#ifndef __EasySFTPFile_FWD_DEFINED__
#define __EasySFTPFile_FWD_DEFINED__

#ifdef __cplusplus
typedef class EasySFTPFile EasySFTPFile;
#else
typedef struct EasySFTPFile EasySFTPFile;
#endif /* __cplusplus */

#endif 	/* __EasySFTPFile_FWD_DEFINED__ */


#ifndef __EasySFTPFiles_FWD_DEFINED__
#define __EasySFTPFiles_FWD_DEFINED__

#ifdef __cplusplus
typedef class EasySFTPFiles EasySFTPFiles;
#else
typedef struct EasySFTPFiles EasySFTPFiles;
#endif /* __cplusplus */

#endif 	/* __EasySFTPFiles_FWD_DEFINED__ */


#ifndef __EasySFTPStream_FWD_DEFINED__
#define __EasySFTPStream_FWD_DEFINED__

#ifdef __cplusplus
typedef class EasySFTPStream EasySFTPStream;
#else
typedef struct EasySFTPStream EasySFTPStream;
#endif /* __cplusplus */

#endif 	/* __EasySFTPStream_FWD_DEFINED__ */


#ifndef __EasySFTPDirectory_FWD_DEFINED__
#define __EasySFTPDirectory_FWD_DEFINED__

#ifdef __cplusplus
typedef class EasySFTPDirectory EasySFTPDirectory;
#else
typedef struct EasySFTPDirectory EasySFTPDirectory;
#endif /* __cplusplus */

#endif 	/* __EasySFTPDirectory_FWD_DEFINED__ */


#ifndef __EasySFTPRootDirectory_FWD_DEFINED__
#define __EasySFTPRootDirectory_FWD_DEFINED__

#ifdef __cplusplus
typedef class EasySFTPRootDirectory EasySFTPRootDirectory;
#else
typedef struct EasySFTPRootDirectory EasySFTPRootDirectory;
#endif /* __cplusplus */

#endif 	/* __EasySFTPRootDirectory_FWD_DEFINED__ */


#ifndef __EasySFTPAuthentication_FWD_DEFINED__
#define __EasySFTPAuthentication_FWD_DEFINED__

#ifdef __cplusplus
typedef class EasySFTPAuthentication EasySFTPAuthentication;
#else
typedef struct EasySFTPAuthentication EasySFTPAuthentication;
#endif /* __cplusplus */

#endif 	/* __EasySFTPAuthentication_FWD_DEFINED__ */


#ifndef __EasySFTPStringList_FWD_DEFINED__
#define __EasySFTPStringList_FWD_DEFINED__

#ifdef __cplusplus
typedef class EasySFTPStringList EasySFTPStringList;
#else
typedef struct EasySFTPStringList EasySFTPStringList;
#endif /* __cplusplus */

#endif 	/* __EasySFTPStringList_FWD_DEFINED__ */


#ifndef __EasySFTPHostSetting_FWD_DEFINED__
#define __EasySFTPHostSetting_FWD_DEFINED__

#ifdef __cplusplus
typedef class EasySFTPHostSetting EasySFTPHostSetting;
#else
typedef struct EasySFTPHostSetting EasySFTPHostSetting;
#endif /* __cplusplus */

#endif 	/* __EasySFTPHostSetting_FWD_DEFINED__ */


#ifndef __EasySFTPHostSettingList_FWD_DEFINED__
#define __EasySFTPHostSettingList_FWD_DEFINED__

#ifdef __cplusplus
typedef class EasySFTPHostSettingList EasySFTPHostSettingList;
#else
typedef struct EasySFTPHostSettingList EasySFTPHostSettingList;
#endif /* __cplusplus */

#endif 	/* __EasySFTPHostSettingList_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"

#ifdef __cplusplus
extern "C"{
#endif 



#ifndef __EasySFTP_LIBRARY_DEFINED__
#define __EasySFTP_LIBRARY_DEFINED__

/* library EasySFTP */
/* [lcid][helpstring][version][uuid] */ 








#define _ENUM_CLASS enum class
#define enum _ENUM_CLASS
typedef 
enum EasySFTPSeekOrigin
    {
        SeekBegin	= 0,
        SeekCurrent	= 1,
        SeekEnd	= 2
    } 	EasySFTPSeekOrigin;

#undef enum
typedef /* [helpstring] */ 
enum EasySFTPTextMode
    {
        NoConversion	= 0,
        BufferCrLf	= 0,
        BufferCr	= 0x1,
        BufferLf	= 0x2,
        BufferMask	= 0x3,
        StreamCrLf	= 0,
        StreamCr	= 0x4,
        StreamLf	= 0x8,
        StreamMask	= 0xc,
        EncodeNone	= 0,
        EncodeShiftJIS	= 0x10,
        EncodeUtf8	= 0x20,
        EncodeEucJp	= 0x30,
        EncodeUcs4	= 0x40,
        EncodeMask	= 0xf0
    } 	EasySFTPTextMode;

#define enum _ENUM_CLASS
typedef 
enum EasySFTPTransferMode
    {
        Auto	= 0,
        Text	= 1,
        Binary	= 2
    } 	EasySFTPTransferMode;

typedef 
enum EasySFTPConnectionMode
    {
        SFTP	= 0,
        FTP	= 1
    } 	EasySFTPConnectionMode;

typedef 
enum EasySFTPAuthenticationMode
    {
        None	= 0,
        Password	= 1,
        PublicKey	= 2,
        Pageant	= 3,
        WinOpenSSH	= 4
    } 	EasySFTPAuthenticationMode;

typedef 
enum EasySFTPServerCharset
    {
        UTF8	= 0,
        ShiftJIS	= 1,
        EUC	= 2
    } 	EasySFTPServerCharset;

#undef enum
#undef _ENUM_CLASS
#define NoConversion EasySFTPTextMode::NoConversion

EXTERN_C const IID LIBID_EasySFTP;

#ifndef __IEasySFTPFile_INTERFACE_DEFINED__
#define __IEasySFTPFile_INTERFACE_DEFINED__

/* interface IEasySFTPFile */
/* [object][dual][unique][uuid] */ 


EXTERN_C const IID IID_IEasySFTPFile;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("AD29C042-B9E3-4648-9DF6-D7DA5B8D0199")
    IEasySFTPFile : public IDispatch
    {
    public:
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_FileName( 
            /* [retval][out] */ BSTR *pFileName) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_IsDirectory( 
            /* [retval][out] */ VARIANT_BOOL *pbRet) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_IsHidden( 
            /* [retval][out] */ VARIANT_BOOL *pbRet) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_IsShortcut( 
            /* [retval][out] */ VARIANT_BOOL *pbRet) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_CreateTime( 
            /* [retval][out] */ DATE *pdRet) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_ModifyTime( 
            /* [retval][out] */ DATE *pdRet) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_Size( 
            /* [retval][out] */ hyper *piRet) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_Size32Bit( 
            /* [retval][out] */ long *piRet) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_Uid( 
            /* [retval][out] */ long *piRet) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_Gid( 
            /* [retval][out] */ long *piRet) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_OwnerName( 
            /* [retval][out] */ BSTR *pRet) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_GroupName( 
            /* [retval][out] */ BSTR *pRet) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_ItemType( 
            /* [retval][out] */ BSTR *pRet) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_Url( 
            /* [retval][out] */ BSTR *pRet) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_Directory( 
            /* [retval][out] */ IEasySFTPDirectory **ppRet) = 0;
        
        virtual /* [hidden][propget][id] */ HRESULT STDMETHODCALLTYPE get_HasWinAttribute( 
            /* [retval][out] */ VARIANT_BOOL *pbRet) = 0;
        
        virtual /* [hidden][propget][id] */ HRESULT STDMETHODCALLTYPE get_WinAttribute( 
            /* [retval][out] */ long *piRet) = 0;
        
        virtual /* [hidden][propget][id] */ HRESULT STDMETHODCALLTYPE get_UnixAttribute( 
            /* [retval][out] */ long *piRet) = 0;
        
    };
    
    
#else 	/* C style interface */

    typedef struct IEasySFTPFileVtbl
    {
        BEGIN_INTERFACE
        
        DECLSPEC_XFGVIRT(IUnknown, QueryInterface)
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEasySFTPFile * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        DECLSPEC_XFGVIRT(IUnknown, AddRef)
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEasySFTPFile * This);
        
        DECLSPEC_XFGVIRT(IUnknown, Release)
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEasySFTPFile * This);
        
        DECLSPEC_XFGVIRT(IDispatch, GetTypeInfoCount)
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IEasySFTPFile * This,
            /* [out] */ UINT *pctinfo);
        
        DECLSPEC_XFGVIRT(IDispatch, GetTypeInfo)
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IEasySFTPFile * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        DECLSPEC_XFGVIRT(IDispatch, GetIDsOfNames)
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IEasySFTPFile * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        DECLSPEC_XFGVIRT(IDispatch, Invoke)
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IEasySFTPFile * This,
            /* [annotation][in] */ 
            _In_  DISPID dispIdMember,
            /* [annotation][in] */ 
            _In_  REFIID riid,
            /* [annotation][in] */ 
            _In_  LCID lcid,
            /* [annotation][in] */ 
            _In_  WORD wFlags,
            /* [annotation][out][in] */ 
            _In_  DISPPARAMS *pDispParams,
            /* [annotation][out] */ 
            _Out_opt_  VARIANT *pVarResult,
            /* [annotation][out] */ 
            _Out_opt_  EXCEPINFO *pExcepInfo,
            /* [annotation][out] */ 
            _Out_opt_  UINT *puArgErr);
        
        DECLSPEC_XFGVIRT(IEasySFTPFile, get_FileName)
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_FileName )( 
            IEasySFTPFile * This,
            /* [retval][out] */ BSTR *pFileName);
        
        DECLSPEC_XFGVIRT(IEasySFTPFile, get_IsDirectory)
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_IsDirectory )( 
            IEasySFTPFile * This,
            /* [retval][out] */ VARIANT_BOOL *pbRet);
        
        DECLSPEC_XFGVIRT(IEasySFTPFile, get_IsHidden)
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_IsHidden )( 
            IEasySFTPFile * This,
            /* [retval][out] */ VARIANT_BOOL *pbRet);
        
        DECLSPEC_XFGVIRT(IEasySFTPFile, get_IsShortcut)
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_IsShortcut )( 
            IEasySFTPFile * This,
            /* [retval][out] */ VARIANT_BOOL *pbRet);
        
        DECLSPEC_XFGVIRT(IEasySFTPFile, get_CreateTime)
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_CreateTime )( 
            IEasySFTPFile * This,
            /* [retval][out] */ DATE *pdRet);
        
        DECLSPEC_XFGVIRT(IEasySFTPFile, get_ModifyTime)
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_ModifyTime )( 
            IEasySFTPFile * This,
            /* [retval][out] */ DATE *pdRet);
        
        DECLSPEC_XFGVIRT(IEasySFTPFile, get_Size)
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_Size )( 
            IEasySFTPFile * This,
            /* [retval][out] */ hyper *piRet);
        
        DECLSPEC_XFGVIRT(IEasySFTPFile, get_Size32Bit)
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_Size32Bit )( 
            IEasySFTPFile * This,
            /* [retval][out] */ long *piRet);
        
        DECLSPEC_XFGVIRT(IEasySFTPFile, get_Uid)
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_Uid )( 
            IEasySFTPFile * This,
            /* [retval][out] */ long *piRet);
        
        DECLSPEC_XFGVIRT(IEasySFTPFile, get_Gid)
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_Gid )( 
            IEasySFTPFile * This,
            /* [retval][out] */ long *piRet);
        
        DECLSPEC_XFGVIRT(IEasySFTPFile, get_OwnerName)
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_OwnerName )( 
            IEasySFTPFile * This,
            /* [retval][out] */ BSTR *pRet);
        
        DECLSPEC_XFGVIRT(IEasySFTPFile, get_GroupName)
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_GroupName )( 
            IEasySFTPFile * This,
            /* [retval][out] */ BSTR *pRet);
        
        DECLSPEC_XFGVIRT(IEasySFTPFile, get_ItemType)
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_ItemType )( 
            IEasySFTPFile * This,
            /* [retval][out] */ BSTR *pRet);
        
        DECLSPEC_XFGVIRT(IEasySFTPFile, get_Url)
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_Url )( 
            IEasySFTPFile * This,
            /* [retval][out] */ BSTR *pRet);
        
        DECLSPEC_XFGVIRT(IEasySFTPFile, get_Directory)
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_Directory )( 
            IEasySFTPFile * This,
            /* [retval][out] */ IEasySFTPDirectory **ppRet);
        
        DECLSPEC_XFGVIRT(IEasySFTPFile, get_HasWinAttribute)
        /* [hidden][propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_HasWinAttribute )( 
            IEasySFTPFile * This,
            /* [retval][out] */ VARIANT_BOOL *pbRet);
        
        DECLSPEC_XFGVIRT(IEasySFTPFile, get_WinAttribute)
        /* [hidden][propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_WinAttribute )( 
            IEasySFTPFile * This,
            /* [retval][out] */ long *piRet);
        
        DECLSPEC_XFGVIRT(IEasySFTPFile, get_UnixAttribute)
        /* [hidden][propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_UnixAttribute )( 
            IEasySFTPFile * This,
            /* [retval][out] */ long *piRet);
        
        END_INTERFACE
    } IEasySFTPFileVtbl;

    interface IEasySFTPFile
    {
        CONST_VTBL struct IEasySFTPFileVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEasySFTPFile_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IEasySFTPFile_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IEasySFTPFile_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IEasySFTPFile_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define IEasySFTPFile_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define IEasySFTPFile_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define IEasySFTPFile_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 


#define IEasySFTPFile_get_FileName(This,pFileName)	\
    ( (This)->lpVtbl -> get_FileName(This,pFileName) ) 

#define IEasySFTPFile_get_IsDirectory(This,pbRet)	\
    ( (This)->lpVtbl -> get_IsDirectory(This,pbRet) ) 

#define IEasySFTPFile_get_IsHidden(This,pbRet)	\
    ( (This)->lpVtbl -> get_IsHidden(This,pbRet) ) 

#define IEasySFTPFile_get_IsShortcut(This,pbRet)	\
    ( (This)->lpVtbl -> get_IsShortcut(This,pbRet) ) 

#define IEasySFTPFile_get_CreateTime(This,pdRet)	\
    ( (This)->lpVtbl -> get_CreateTime(This,pdRet) ) 

#define IEasySFTPFile_get_ModifyTime(This,pdRet)	\
    ( (This)->lpVtbl -> get_ModifyTime(This,pdRet) ) 

#define IEasySFTPFile_get_Size(This,piRet)	\
    ( (This)->lpVtbl -> get_Size(This,piRet) ) 

#define IEasySFTPFile_get_Size32Bit(This,piRet)	\
    ( (This)->lpVtbl -> get_Size32Bit(This,piRet) ) 

#define IEasySFTPFile_get_Uid(This,piRet)	\
    ( (This)->lpVtbl -> get_Uid(This,piRet) ) 

#define IEasySFTPFile_get_Gid(This,piRet)	\
    ( (This)->lpVtbl -> get_Gid(This,piRet) ) 

#define IEasySFTPFile_get_OwnerName(This,pRet)	\
    ( (This)->lpVtbl -> get_OwnerName(This,pRet) ) 

#define IEasySFTPFile_get_GroupName(This,pRet)	\
    ( (This)->lpVtbl -> get_GroupName(This,pRet) ) 

#define IEasySFTPFile_get_ItemType(This,pRet)	\
    ( (This)->lpVtbl -> get_ItemType(This,pRet) ) 

#define IEasySFTPFile_get_Url(This,pRet)	\
    ( (This)->lpVtbl -> get_Url(This,pRet) ) 

#define IEasySFTPFile_get_Directory(This,ppRet)	\
    ( (This)->lpVtbl -> get_Directory(This,ppRet) ) 

#define IEasySFTPFile_get_HasWinAttribute(This,pbRet)	\
    ( (This)->lpVtbl -> get_HasWinAttribute(This,pbRet) ) 

#define IEasySFTPFile_get_WinAttribute(This,piRet)	\
    ( (This)->lpVtbl -> get_WinAttribute(This,piRet) ) 

#define IEasySFTPFile_get_UnixAttribute(This,piRet)	\
    ( (This)->lpVtbl -> get_UnixAttribute(This,piRet) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IEasySFTPFile_INTERFACE_DEFINED__ */


#ifndef __IEasySFTPFiles_INTERFACE_DEFINED__
#define __IEasySFTPFiles_INTERFACE_DEFINED__

/* interface IEasySFTPFiles */
/* [object][dual][unique][uuid] */ 


EXTERN_C const IID IID_IEasySFTPFiles;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("AD29C042-B9E3-464a-9DF6-D7DA5B8D0199")
    IEasySFTPFiles : public IDispatch
    {
    public:
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_Item( 
            /* [in] */ VARIANT key,
            /* [retval][out] */ IEasySFTPFile **ppFile) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_Count( 
            /* [retval][out] */ long *pRet) = 0;
        
        virtual /* [hidden][propget][id] */ HRESULT STDMETHODCALLTYPE get__NewEnum( 
            /* [retval][out] */ IUnknown **ppRet) = 0;
        
    };
    
    
#else 	/* C style interface */

    typedef struct IEasySFTPFilesVtbl
    {
        BEGIN_INTERFACE
        
        DECLSPEC_XFGVIRT(IUnknown, QueryInterface)
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEasySFTPFiles * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        DECLSPEC_XFGVIRT(IUnknown, AddRef)
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEasySFTPFiles * This);
        
        DECLSPEC_XFGVIRT(IUnknown, Release)
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEasySFTPFiles * This);
        
        DECLSPEC_XFGVIRT(IDispatch, GetTypeInfoCount)
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IEasySFTPFiles * This,
            /* [out] */ UINT *pctinfo);
        
        DECLSPEC_XFGVIRT(IDispatch, GetTypeInfo)
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IEasySFTPFiles * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        DECLSPEC_XFGVIRT(IDispatch, GetIDsOfNames)
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IEasySFTPFiles * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        DECLSPEC_XFGVIRT(IDispatch, Invoke)
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IEasySFTPFiles * This,
            /* [annotation][in] */ 
            _In_  DISPID dispIdMember,
            /* [annotation][in] */ 
            _In_  REFIID riid,
            /* [annotation][in] */ 
            _In_  LCID lcid,
            /* [annotation][in] */ 
            _In_  WORD wFlags,
            /* [annotation][out][in] */ 
            _In_  DISPPARAMS *pDispParams,
            /* [annotation][out] */ 
            _Out_opt_  VARIANT *pVarResult,
            /* [annotation][out] */ 
            _Out_opt_  EXCEPINFO *pExcepInfo,
            /* [annotation][out] */ 
            _Out_opt_  UINT *puArgErr);
        
        DECLSPEC_XFGVIRT(IEasySFTPFiles, get_Item)
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_Item )( 
            IEasySFTPFiles * This,
            /* [in] */ VARIANT key,
            /* [retval][out] */ IEasySFTPFile **ppFile);
        
        DECLSPEC_XFGVIRT(IEasySFTPFiles, get_Count)
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            IEasySFTPFiles * This,
            /* [retval][out] */ long *pRet);
        
        DECLSPEC_XFGVIRT(IEasySFTPFiles, get__NewEnum)
        /* [hidden][propget][id] */ HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            IEasySFTPFiles * This,
            /* [retval][out] */ IUnknown **ppRet);
        
        END_INTERFACE
    } IEasySFTPFilesVtbl;

    interface IEasySFTPFiles
    {
        CONST_VTBL struct IEasySFTPFilesVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEasySFTPFiles_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IEasySFTPFiles_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IEasySFTPFiles_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IEasySFTPFiles_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define IEasySFTPFiles_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define IEasySFTPFiles_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define IEasySFTPFiles_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 


#define IEasySFTPFiles_get_Item(This,key,ppFile)	\
    ( (This)->lpVtbl -> get_Item(This,key,ppFile) ) 

#define IEasySFTPFiles_get_Count(This,pRet)	\
    ( (This)->lpVtbl -> get_Count(This,pRet) ) 

#define IEasySFTPFiles_get__NewEnum(This,ppRet)	\
    ( (This)->lpVtbl -> get__NewEnum(This,ppRet) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IEasySFTPFiles_INTERFACE_DEFINED__ */


#ifndef __IEasySFTPStream_INTERFACE_DEFINED__
#define __IEasySFTPStream_INTERFACE_DEFINED__

/* interface IEasySFTPStream */
/* [object][dual][unique][uuid] */ 


EXTERN_C const IID IID_IEasySFTPStream;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("AD29C042-B9E3-464c-9DF6-D7DA5B8D0199")
    IEasySFTPStream : public IDispatch
    {
    public:
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE Read( 
            /* [out] */ void *buffer,
            /* [in] */ long length,
            /* [retval][out] */ long *piRead) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE Write( 
            /* [in] */ const void *buffer,
            /* [in] */ long length,
            /* [retval][out] */ long *piWritten) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE ReadAsUtf8String( 
            /* [in] */ long length,
            /* [retval][out] */ BSTR *pbRet) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE WriteAsUtf8String( 
            /* [in] */ BSTR bstrString,
            /* [retval][out] */ long *piWritten) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE Seek( 
            /* [in] */ long pos,
            /* [in] */ EasySFTPSeekOrigin origin,
            /* [retval][out] */ long *pNew) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE Seek64( 
            /* [in] */ hyper pos,
            /* [in] */ EasySFTPSeekOrigin origin,
            /* [retval][out] */ hyper *pNew) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_File( 
            /* [retval][out] */ IEasySFTPFile **ppFile) = 0;
        
    };
    
    
#else 	/* C style interface */

    typedef struct IEasySFTPStreamVtbl
    {
        BEGIN_INTERFACE
        
        DECLSPEC_XFGVIRT(IUnknown, QueryInterface)
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEasySFTPStream * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        DECLSPEC_XFGVIRT(IUnknown, AddRef)
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEasySFTPStream * This);
        
        DECLSPEC_XFGVIRT(IUnknown, Release)
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEasySFTPStream * This);
        
        DECLSPEC_XFGVIRT(IDispatch, GetTypeInfoCount)
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IEasySFTPStream * This,
            /* [out] */ UINT *pctinfo);
        
        DECLSPEC_XFGVIRT(IDispatch, GetTypeInfo)
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IEasySFTPStream * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        DECLSPEC_XFGVIRT(IDispatch, GetIDsOfNames)
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IEasySFTPStream * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        DECLSPEC_XFGVIRT(IDispatch, Invoke)
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IEasySFTPStream * This,
            /* [annotation][in] */ 
            _In_  DISPID dispIdMember,
            /* [annotation][in] */ 
            _In_  REFIID riid,
            /* [annotation][in] */ 
            _In_  LCID lcid,
            /* [annotation][in] */ 
            _In_  WORD wFlags,
            /* [annotation][out][in] */ 
            _In_  DISPPARAMS *pDispParams,
            /* [annotation][out] */ 
            _Out_opt_  VARIANT *pVarResult,
            /* [annotation][out] */ 
            _Out_opt_  EXCEPINFO *pExcepInfo,
            /* [annotation][out] */ 
            _Out_opt_  UINT *puArgErr);
        
        DECLSPEC_XFGVIRT(IEasySFTPStream, Read)
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *Read )( 
            IEasySFTPStream * This,
            /* [out] */ void *buffer,
            /* [in] */ long length,
            /* [retval][out] */ long *piRead);
        
        DECLSPEC_XFGVIRT(IEasySFTPStream, Write)
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *Write )( 
            IEasySFTPStream * This,
            /* [in] */ const void *buffer,
            /* [in] */ long length,
            /* [retval][out] */ long *piWritten);
        
        DECLSPEC_XFGVIRT(IEasySFTPStream, ReadAsUtf8String)
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *ReadAsUtf8String )( 
            IEasySFTPStream * This,
            /* [in] */ long length,
            /* [retval][out] */ BSTR *pbRet);
        
        DECLSPEC_XFGVIRT(IEasySFTPStream, WriteAsUtf8String)
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *WriteAsUtf8String )( 
            IEasySFTPStream * This,
            /* [in] */ BSTR bstrString,
            /* [retval][out] */ long *piWritten);
        
        DECLSPEC_XFGVIRT(IEasySFTPStream, Seek)
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *Seek )( 
            IEasySFTPStream * This,
            /* [in] */ long pos,
            /* [in] */ EasySFTPSeekOrigin origin,
            /* [retval][out] */ long *pNew);
        
        DECLSPEC_XFGVIRT(IEasySFTPStream, Seek64)
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *Seek64 )( 
            IEasySFTPStream * This,
            /* [in] */ hyper pos,
            /* [in] */ EasySFTPSeekOrigin origin,
            /* [retval][out] */ hyper *pNew);
        
        DECLSPEC_XFGVIRT(IEasySFTPStream, get_File)
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_File )( 
            IEasySFTPStream * This,
            /* [retval][out] */ IEasySFTPFile **ppFile);
        
        END_INTERFACE
    } IEasySFTPStreamVtbl;

    interface IEasySFTPStream
    {
        CONST_VTBL struct IEasySFTPStreamVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEasySFTPStream_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IEasySFTPStream_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IEasySFTPStream_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IEasySFTPStream_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define IEasySFTPStream_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define IEasySFTPStream_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define IEasySFTPStream_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 


#define IEasySFTPStream_Read(This,buffer,length,piRead)	\
    ( (This)->lpVtbl -> Read(This,buffer,length,piRead) ) 

#define IEasySFTPStream_Write(This,buffer,length,piWritten)	\
    ( (This)->lpVtbl -> Write(This,buffer,length,piWritten) ) 

#define IEasySFTPStream_ReadAsUtf8String(This,length,pbRet)	\
    ( (This)->lpVtbl -> ReadAsUtf8String(This,length,pbRet) ) 

#define IEasySFTPStream_WriteAsUtf8String(This,bstrString,piWritten)	\
    ( (This)->lpVtbl -> WriteAsUtf8String(This,bstrString,piWritten) ) 

#define IEasySFTPStream_Seek(This,pos,origin,pNew)	\
    ( (This)->lpVtbl -> Seek(This,pos,origin,pNew) ) 

#define IEasySFTPStream_Seek64(This,pos,origin,pNew)	\
    ( (This)->lpVtbl -> Seek64(This,pos,origin,pNew) ) 

#define IEasySFTPStream_get_File(This,ppFile)	\
    ( (This)->lpVtbl -> get_File(This,ppFile) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IEasySFTPStream_INTERFACE_DEFINED__ */


#ifndef __IEasySFTPDirectory_INTERFACE_DEFINED__
#define __IEasySFTPDirectory_INTERFACE_DEFINED__

/* interface IEasySFTPDirectory */
/* [object][dual][unique][uuid] */ 


EXTERN_C const IID IID_IEasySFTPDirectory;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("AD29C042-B9E3-4646-9DF6-D7DA5B8D0199")
    IEasySFTPDirectory : public IDispatch
    {
    public:
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_Name( 
            /* [retval][out] */ BSTR *pRet) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_RootDirectory( 
            /* [retval][out] */ IEasySFTPRootDirectory **ppRootDirectory) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE OpenDirectory( 
            /* [in] */ VARIANT File,
            /* [retval][out] */ IEasySFTPDirectory **ppRet) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_Files( 
            /* [retval][out] */ IEasySFTPFiles **ppFiles) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE OpenTransferDialog( 
            /* [in] */ LONG_PTR hWndOwner) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE CloseTransferDialog( void) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE OpenFile( 
            /* [in] */ BSTR lpszRelativeFileName,
            /* [in] */ VARIANT_BOOL bIsWrite,
            /* [defaultvalue][in] */ EasySFTPTextMode nTextMode,
            /* [retval][out] */ IEasySFTPStream **ppStream) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE UploadFrom( 
            /* [in] */ BSTR lpszDestinationRelativeName,
            /* [in] */ BSTR lpszSourceLocalName,
            /* [defaultvalue][in] */ EasySFTPTextMode nTextMode = NoConversion) = 0;
        
        virtual /* [hidden][id] */ HRESULT STDMETHODCALLTYPE UploadFromStream( 
            /* [in] */ BSTR lpszDestinationRelativeFileName,
            /* [in] */ IUnknown *pStream,
            /* [defaultvalue][in] */ EasySFTPTextMode nTextMode = NoConversion) = 0;
        
        virtual /* [hidden][id] */ HRESULT STDMETHODCALLTYPE UploadFromDataObject( 
            /* [in] */ IUnknown *pObject,
            /* [defaultvalue][in] */ EasySFTPTextMode nTextMode = NoConversion) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE DownloadTo( 
            /* [in] */ VARIANT File,
            /* [in] */ BSTR lpszTargetLocalName,
            /* [defaultvalue][in] */ EasySFTPTextMode nTextMode = NoConversion) = 0;
        
        virtual /* [hidden][id] */ HRESULT STDMETHODCALLTYPE DownloadToStream( 
            /* [in] */ VARIANT File,
            /* [in] */ IUnknown *pStream,
            /* [defaultvalue][in] */ EasySFTPTextMode nTextMode = NoConversion) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE UploadFiles( 
            /* [in] */ SAFEARRAY * LocalFiles,
            /* [defaultvalue][in] */ EasySFTPTextMode nTextMode = NoConversion) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE DownloadFiles( 
            /* [in] */ SAFEARRAY * RemoteFiles,
            /* [in] */ BSTR bstrDestinationDirectory,
            /* [defaultvalue][in] */ EasySFTPTextMode nTextMode = NoConversion) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE Move( 
            /* [in] */ VARIANT File,
            /* [in] */ BSTR lpszTargetName) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE Remove( 
            /* [in] */ VARIANT File) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE UpdateFileTime( 
            /* [in] */ VARIANT File,
            /* [in] */ DATE modifyTime,
            /* [in] */ DATE createTime,
            /* [defaultvalue][in] */ DATE accessTime = 0) = 0;
        
        virtual /* [hidden][id] */ HRESULT STDMETHODCALLTYPE UpdateAttributes( 
            /* [in] */ VARIANT File,
            /* [in] */ long attr) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE CreateShortcut( 
            /* [in] */ BSTR LinkName,
            /* [in] */ BSTR TargetName) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_FullPath( 
            /* [retval][out] */ BSTR *pRet) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_Url( 
            /* [retval][out] */ BSTR *pRet) = 0;
        
    };
    
    
#else 	/* C style interface */

    typedef struct IEasySFTPDirectoryVtbl
    {
        BEGIN_INTERFACE
        
        DECLSPEC_XFGVIRT(IUnknown, QueryInterface)
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEasySFTPDirectory * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        DECLSPEC_XFGVIRT(IUnknown, AddRef)
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEasySFTPDirectory * This);
        
        DECLSPEC_XFGVIRT(IUnknown, Release)
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEasySFTPDirectory * This);
        
        DECLSPEC_XFGVIRT(IDispatch, GetTypeInfoCount)
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IEasySFTPDirectory * This,
            /* [out] */ UINT *pctinfo);
        
        DECLSPEC_XFGVIRT(IDispatch, GetTypeInfo)
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IEasySFTPDirectory * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        DECLSPEC_XFGVIRT(IDispatch, GetIDsOfNames)
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IEasySFTPDirectory * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        DECLSPEC_XFGVIRT(IDispatch, Invoke)
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IEasySFTPDirectory * This,
            /* [annotation][in] */ 
            _In_  DISPID dispIdMember,
            /* [annotation][in] */ 
            _In_  REFIID riid,
            /* [annotation][in] */ 
            _In_  LCID lcid,
            /* [annotation][in] */ 
            _In_  WORD wFlags,
            /* [annotation][out][in] */ 
            _In_  DISPPARAMS *pDispParams,
            /* [annotation][out] */ 
            _Out_opt_  VARIANT *pVarResult,
            /* [annotation][out] */ 
            _Out_opt_  EXCEPINFO *pExcepInfo,
            /* [annotation][out] */ 
            _Out_opt_  UINT *puArgErr);
        
        DECLSPEC_XFGVIRT(IEasySFTPDirectory, get_Name)
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            IEasySFTPDirectory * This,
            /* [retval][out] */ BSTR *pRet);
        
        DECLSPEC_XFGVIRT(IEasySFTPDirectory, get_RootDirectory)
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_RootDirectory )( 
            IEasySFTPDirectory * This,
            /* [retval][out] */ IEasySFTPRootDirectory **ppRootDirectory);
        
        DECLSPEC_XFGVIRT(IEasySFTPDirectory, OpenDirectory)
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *OpenDirectory )( 
            IEasySFTPDirectory * This,
            /* [in] */ VARIANT File,
            /* [retval][out] */ IEasySFTPDirectory **ppRet);
        
        DECLSPEC_XFGVIRT(IEasySFTPDirectory, get_Files)
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_Files )( 
            IEasySFTPDirectory * This,
            /* [retval][out] */ IEasySFTPFiles **ppFiles);
        
        DECLSPEC_XFGVIRT(IEasySFTPDirectory, OpenTransferDialog)
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *OpenTransferDialog )( 
            IEasySFTPDirectory * This,
            /* [in] */ LONG_PTR hWndOwner);
        
        DECLSPEC_XFGVIRT(IEasySFTPDirectory, CloseTransferDialog)
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *CloseTransferDialog )( 
            IEasySFTPDirectory * This);
        
        DECLSPEC_XFGVIRT(IEasySFTPDirectory, OpenFile)
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *OpenFile )( 
            IEasySFTPDirectory * This,
            /* [in] */ BSTR lpszRelativeFileName,
            /* [in] */ VARIANT_BOOL bIsWrite,
            /* [defaultvalue][in] */ EasySFTPTextMode nTextMode,
            /* [retval][out] */ IEasySFTPStream **ppStream);
        
        DECLSPEC_XFGVIRT(IEasySFTPDirectory, UploadFrom)
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *UploadFrom )( 
            IEasySFTPDirectory * This,
            /* [in] */ BSTR lpszDestinationRelativeName,
            /* [in] */ BSTR lpszSourceLocalName,
            /* [defaultvalue][in] */ EasySFTPTextMode nTextMode);
        
        DECLSPEC_XFGVIRT(IEasySFTPDirectory, UploadFromStream)
        /* [hidden][id] */ HRESULT ( STDMETHODCALLTYPE *UploadFromStream )( 
            IEasySFTPDirectory * This,
            /* [in] */ BSTR lpszDestinationRelativeFileName,
            /* [in] */ IUnknown *pStream,
            /* [defaultvalue][in] */ EasySFTPTextMode nTextMode);
        
        DECLSPEC_XFGVIRT(IEasySFTPDirectory, UploadFromDataObject)
        /* [hidden][id] */ HRESULT ( STDMETHODCALLTYPE *UploadFromDataObject )( 
            IEasySFTPDirectory * This,
            /* [in] */ IUnknown *pObject,
            /* [defaultvalue][in] */ EasySFTPTextMode nTextMode);
        
        DECLSPEC_XFGVIRT(IEasySFTPDirectory, DownloadTo)
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *DownloadTo )( 
            IEasySFTPDirectory * This,
            /* [in] */ VARIANT File,
            /* [in] */ BSTR lpszTargetLocalName,
            /* [defaultvalue][in] */ EasySFTPTextMode nTextMode);
        
        DECLSPEC_XFGVIRT(IEasySFTPDirectory, DownloadToStream)
        /* [hidden][id] */ HRESULT ( STDMETHODCALLTYPE *DownloadToStream )( 
            IEasySFTPDirectory * This,
            /* [in] */ VARIANT File,
            /* [in] */ IUnknown *pStream,
            /* [defaultvalue][in] */ EasySFTPTextMode nTextMode);
        
        DECLSPEC_XFGVIRT(IEasySFTPDirectory, UploadFiles)
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *UploadFiles )( 
            IEasySFTPDirectory * This,
            /* [in] */ SAFEARRAY * LocalFiles,
            /* [defaultvalue][in] */ EasySFTPTextMode nTextMode);
        
        DECLSPEC_XFGVIRT(IEasySFTPDirectory, DownloadFiles)
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *DownloadFiles )( 
            IEasySFTPDirectory * This,
            /* [in] */ SAFEARRAY * RemoteFiles,
            /* [in] */ BSTR bstrDestinationDirectory,
            /* [defaultvalue][in] */ EasySFTPTextMode nTextMode);
        
        DECLSPEC_XFGVIRT(IEasySFTPDirectory, Move)
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *Move )( 
            IEasySFTPDirectory * This,
            /* [in] */ VARIANT File,
            /* [in] */ BSTR lpszTargetName);
        
        DECLSPEC_XFGVIRT(IEasySFTPDirectory, Remove)
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *Remove )( 
            IEasySFTPDirectory * This,
            /* [in] */ VARIANT File);
        
        DECLSPEC_XFGVIRT(IEasySFTPDirectory, UpdateFileTime)
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *UpdateFileTime )( 
            IEasySFTPDirectory * This,
            /* [in] */ VARIANT File,
            /* [in] */ DATE modifyTime,
            /* [in] */ DATE createTime,
            /* [defaultvalue][in] */ DATE accessTime);
        
        DECLSPEC_XFGVIRT(IEasySFTPDirectory, UpdateAttributes)
        /* [hidden][id] */ HRESULT ( STDMETHODCALLTYPE *UpdateAttributes )( 
            IEasySFTPDirectory * This,
            /* [in] */ VARIANT File,
            /* [in] */ long attr);
        
        DECLSPEC_XFGVIRT(IEasySFTPDirectory, CreateShortcut)
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *CreateShortcut )( 
            IEasySFTPDirectory * This,
            /* [in] */ BSTR LinkName,
            /* [in] */ BSTR TargetName);
        
        DECLSPEC_XFGVIRT(IEasySFTPDirectory, get_FullPath)
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_FullPath )( 
            IEasySFTPDirectory * This,
            /* [retval][out] */ BSTR *pRet);
        
        DECLSPEC_XFGVIRT(IEasySFTPDirectory, get_Url)
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_Url )( 
            IEasySFTPDirectory * This,
            /* [retval][out] */ BSTR *pRet);
        
        END_INTERFACE
    } IEasySFTPDirectoryVtbl;

    interface IEasySFTPDirectory
    {
        CONST_VTBL struct IEasySFTPDirectoryVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEasySFTPDirectory_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IEasySFTPDirectory_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IEasySFTPDirectory_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IEasySFTPDirectory_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define IEasySFTPDirectory_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define IEasySFTPDirectory_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define IEasySFTPDirectory_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 


#define IEasySFTPDirectory_get_Name(This,pRet)	\
    ( (This)->lpVtbl -> get_Name(This,pRet) ) 

#define IEasySFTPDirectory_get_RootDirectory(This,ppRootDirectory)	\
    ( (This)->lpVtbl -> get_RootDirectory(This,ppRootDirectory) ) 

#define IEasySFTPDirectory_OpenDirectory(This,File,ppRet)	\
    ( (This)->lpVtbl -> OpenDirectory(This,File,ppRet) ) 

#define IEasySFTPDirectory_get_Files(This,ppFiles)	\
    ( (This)->lpVtbl -> get_Files(This,ppFiles) ) 

#define IEasySFTPDirectory_OpenTransferDialog(This,hWndOwner)	\
    ( (This)->lpVtbl -> OpenTransferDialog(This,hWndOwner) ) 

#define IEasySFTPDirectory_CloseTransferDialog(This)	\
    ( (This)->lpVtbl -> CloseTransferDialog(This) ) 

#define IEasySFTPDirectory_OpenFile(This,lpszRelativeFileName,bIsWrite,nTextMode,ppStream)	\
    ( (This)->lpVtbl -> OpenFile(This,lpszRelativeFileName,bIsWrite,nTextMode,ppStream) ) 

#define IEasySFTPDirectory_UploadFrom(This,lpszDestinationRelativeName,lpszSourceLocalName,nTextMode)	\
    ( (This)->lpVtbl -> UploadFrom(This,lpszDestinationRelativeName,lpszSourceLocalName,nTextMode) ) 

#define IEasySFTPDirectory_UploadFromStream(This,lpszDestinationRelativeFileName,pStream,nTextMode)	\
    ( (This)->lpVtbl -> UploadFromStream(This,lpszDestinationRelativeFileName,pStream,nTextMode) ) 

#define IEasySFTPDirectory_UploadFromDataObject(This,pObject,nTextMode)	\
    ( (This)->lpVtbl -> UploadFromDataObject(This,pObject,nTextMode) ) 

#define IEasySFTPDirectory_DownloadTo(This,File,lpszTargetLocalName,nTextMode)	\
    ( (This)->lpVtbl -> DownloadTo(This,File,lpszTargetLocalName,nTextMode) ) 

#define IEasySFTPDirectory_DownloadToStream(This,File,pStream,nTextMode)	\
    ( (This)->lpVtbl -> DownloadToStream(This,File,pStream,nTextMode) ) 

#define IEasySFTPDirectory_UploadFiles(This,LocalFiles,nTextMode)	\
    ( (This)->lpVtbl -> UploadFiles(This,LocalFiles,nTextMode) ) 

#define IEasySFTPDirectory_DownloadFiles(This,RemoteFiles,bstrDestinationDirectory,nTextMode)	\
    ( (This)->lpVtbl -> DownloadFiles(This,RemoteFiles,bstrDestinationDirectory,nTextMode) ) 

#define IEasySFTPDirectory_Move(This,File,lpszTargetName)	\
    ( (This)->lpVtbl -> Move(This,File,lpszTargetName) ) 

#define IEasySFTPDirectory_Remove(This,File)	\
    ( (This)->lpVtbl -> Remove(This,File) ) 

#define IEasySFTPDirectory_UpdateFileTime(This,File,modifyTime,createTime,accessTime)	\
    ( (This)->lpVtbl -> UpdateFileTime(This,File,modifyTime,createTime,accessTime) ) 

#define IEasySFTPDirectory_UpdateAttributes(This,File,attr)	\
    ( (This)->lpVtbl -> UpdateAttributes(This,File,attr) ) 

#define IEasySFTPDirectory_CreateShortcut(This,LinkName,TargetName)	\
    ( (This)->lpVtbl -> CreateShortcut(This,LinkName,TargetName) ) 

#define IEasySFTPDirectory_get_FullPath(This,pRet)	\
    ( (This)->lpVtbl -> get_FullPath(This,pRet) ) 

#define IEasySFTPDirectory_get_Url(This,pRet)	\
    ( (This)->lpVtbl -> get_Url(This,pRet) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IEasySFTPDirectory_INTERFACE_DEFINED__ */


#ifndef __IEasySFTPRootDirectory_INTERFACE_DEFINED__
#define __IEasySFTPRootDirectory_INTERFACE_DEFINED__

/* interface IEasySFTPRootDirectory */
/* [object][dual][unique][uuid] */ 


EXTERN_C const IID IID_IEasySFTPRootDirectory;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("AD29C042-B9E3-464e-9DF6-D7DA5B8D0199")
    IEasySFTPRootDirectory : public IEasySFTPDirectory
    {
    public:
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_IsUnixServer( 
            /* [retval][out] */ VARIANT_BOOL *pbRet) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_HostName( 
            /* [retval][out] */ BSTR *pRet) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_Port( 
            /* [retval][out] */ long *pRet) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_ConnectionMode( 
            /* [retval][out] */ EasySFTPConnectionMode *pRet) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_TextMode( 
            /* [retval][out] */ EasySFTPTextMode *pnTextMode) = 0;
        
        virtual /* [propput][id] */ HRESULT STDMETHODCALLTYPE put_TextMode( 
            /* [in] */ EasySFTPTextMode nTextMode) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_TransferMode( 
            /* [retval][out] */ EasySFTPTransferMode *pnTransferMode) = 0;
        
        virtual /* [propput][id] */ HRESULT STDMETHODCALLTYPE put_TransferMode( 
            /* [in] */ EasySFTPTransferMode nTransferMode) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE IsTextFile( 
            /* [in] */ BSTR lpszFileName,
            /* [retval][out] */ VARIANT_BOOL *pbRet) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE Disconnect( void) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_Connected( 
            /* [retval][out] */ VARIANT_BOOL *pbRet) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_Transferring( 
            /* [retval][out] */ VARIANT_BOOL *pbRet) = 0;
        
    };
    
    
#else 	/* C style interface */

    typedef struct IEasySFTPRootDirectoryVtbl
    {
        BEGIN_INTERFACE
        
        DECLSPEC_XFGVIRT(IUnknown, QueryInterface)
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEasySFTPRootDirectory * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        DECLSPEC_XFGVIRT(IUnknown, AddRef)
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEasySFTPRootDirectory * This);
        
        DECLSPEC_XFGVIRT(IUnknown, Release)
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEasySFTPRootDirectory * This);
        
        DECLSPEC_XFGVIRT(IDispatch, GetTypeInfoCount)
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IEasySFTPRootDirectory * This,
            /* [out] */ UINT *pctinfo);
        
        DECLSPEC_XFGVIRT(IDispatch, GetTypeInfo)
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IEasySFTPRootDirectory * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        DECLSPEC_XFGVIRT(IDispatch, GetIDsOfNames)
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IEasySFTPRootDirectory * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        DECLSPEC_XFGVIRT(IDispatch, Invoke)
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IEasySFTPRootDirectory * This,
            /* [annotation][in] */ 
            _In_  DISPID dispIdMember,
            /* [annotation][in] */ 
            _In_  REFIID riid,
            /* [annotation][in] */ 
            _In_  LCID lcid,
            /* [annotation][in] */ 
            _In_  WORD wFlags,
            /* [annotation][out][in] */ 
            _In_  DISPPARAMS *pDispParams,
            /* [annotation][out] */ 
            _Out_opt_  VARIANT *pVarResult,
            /* [annotation][out] */ 
            _Out_opt_  EXCEPINFO *pExcepInfo,
            /* [annotation][out] */ 
            _Out_opt_  UINT *puArgErr);
        
        DECLSPEC_XFGVIRT(IEasySFTPDirectory, get_Name)
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            IEasySFTPRootDirectory * This,
            /* [retval][out] */ BSTR *pRet);
        
        DECLSPEC_XFGVIRT(IEasySFTPDirectory, get_RootDirectory)
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_RootDirectory )( 
            IEasySFTPRootDirectory * This,
            /* [retval][out] */ IEasySFTPRootDirectory **ppRootDirectory);
        
        DECLSPEC_XFGVIRT(IEasySFTPDirectory, OpenDirectory)
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *OpenDirectory )( 
            IEasySFTPRootDirectory * This,
            /* [in] */ VARIANT File,
            /* [retval][out] */ IEasySFTPDirectory **ppRet);
        
        DECLSPEC_XFGVIRT(IEasySFTPDirectory, get_Files)
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_Files )( 
            IEasySFTPRootDirectory * This,
            /* [retval][out] */ IEasySFTPFiles **ppFiles);
        
        DECLSPEC_XFGVIRT(IEasySFTPDirectory, OpenTransferDialog)
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *OpenTransferDialog )( 
            IEasySFTPRootDirectory * This,
            /* [in] */ LONG_PTR hWndOwner);
        
        DECLSPEC_XFGVIRT(IEasySFTPDirectory, CloseTransferDialog)
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *CloseTransferDialog )( 
            IEasySFTPRootDirectory * This);
        
        DECLSPEC_XFGVIRT(IEasySFTPDirectory, OpenFile)
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *OpenFile )( 
            IEasySFTPRootDirectory * This,
            /* [in] */ BSTR lpszRelativeFileName,
            /* [in] */ VARIANT_BOOL bIsWrite,
            /* [defaultvalue][in] */ EasySFTPTextMode nTextMode,
            /* [retval][out] */ IEasySFTPStream **ppStream);
        
        DECLSPEC_XFGVIRT(IEasySFTPDirectory, UploadFrom)
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *UploadFrom )( 
            IEasySFTPRootDirectory * This,
            /* [in] */ BSTR lpszDestinationRelativeName,
            /* [in] */ BSTR lpszSourceLocalName,
            /* [defaultvalue][in] */ EasySFTPTextMode nTextMode);
        
        DECLSPEC_XFGVIRT(IEasySFTPDirectory, UploadFromStream)
        /* [hidden][id] */ HRESULT ( STDMETHODCALLTYPE *UploadFromStream )( 
            IEasySFTPRootDirectory * This,
            /* [in] */ BSTR lpszDestinationRelativeFileName,
            /* [in] */ IUnknown *pStream,
            /* [defaultvalue][in] */ EasySFTPTextMode nTextMode);
        
        DECLSPEC_XFGVIRT(IEasySFTPDirectory, UploadFromDataObject)
        /* [hidden][id] */ HRESULT ( STDMETHODCALLTYPE *UploadFromDataObject )( 
            IEasySFTPRootDirectory * This,
            /* [in] */ IUnknown *pObject,
            /* [defaultvalue][in] */ EasySFTPTextMode nTextMode);
        
        DECLSPEC_XFGVIRT(IEasySFTPDirectory, DownloadTo)
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *DownloadTo )( 
            IEasySFTPRootDirectory * This,
            /* [in] */ VARIANT File,
            /* [in] */ BSTR lpszTargetLocalName,
            /* [defaultvalue][in] */ EasySFTPTextMode nTextMode);
        
        DECLSPEC_XFGVIRT(IEasySFTPDirectory, DownloadToStream)
        /* [hidden][id] */ HRESULT ( STDMETHODCALLTYPE *DownloadToStream )( 
            IEasySFTPRootDirectory * This,
            /* [in] */ VARIANT File,
            /* [in] */ IUnknown *pStream,
            /* [defaultvalue][in] */ EasySFTPTextMode nTextMode);
        
        DECLSPEC_XFGVIRT(IEasySFTPDirectory, UploadFiles)
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *UploadFiles )( 
            IEasySFTPRootDirectory * This,
            /* [in] */ SAFEARRAY * LocalFiles,
            /* [defaultvalue][in] */ EasySFTPTextMode nTextMode);
        
        DECLSPEC_XFGVIRT(IEasySFTPDirectory, DownloadFiles)
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *DownloadFiles )( 
            IEasySFTPRootDirectory * This,
            /* [in] */ SAFEARRAY * RemoteFiles,
            /* [in] */ BSTR bstrDestinationDirectory,
            /* [defaultvalue][in] */ EasySFTPTextMode nTextMode);
        
        DECLSPEC_XFGVIRT(IEasySFTPDirectory, Move)
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *Move )( 
            IEasySFTPRootDirectory * This,
            /* [in] */ VARIANT File,
            /* [in] */ BSTR lpszTargetName);
        
        DECLSPEC_XFGVIRT(IEasySFTPDirectory, Remove)
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *Remove )( 
            IEasySFTPRootDirectory * This,
            /* [in] */ VARIANT File);
        
        DECLSPEC_XFGVIRT(IEasySFTPDirectory, UpdateFileTime)
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *UpdateFileTime )( 
            IEasySFTPRootDirectory * This,
            /* [in] */ VARIANT File,
            /* [in] */ DATE modifyTime,
            /* [in] */ DATE createTime,
            /* [defaultvalue][in] */ DATE accessTime);
        
        DECLSPEC_XFGVIRT(IEasySFTPDirectory, UpdateAttributes)
        /* [hidden][id] */ HRESULT ( STDMETHODCALLTYPE *UpdateAttributes )( 
            IEasySFTPRootDirectory * This,
            /* [in] */ VARIANT File,
            /* [in] */ long attr);
        
        DECLSPEC_XFGVIRT(IEasySFTPDirectory, CreateShortcut)
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *CreateShortcut )( 
            IEasySFTPRootDirectory * This,
            /* [in] */ BSTR LinkName,
            /* [in] */ BSTR TargetName);
        
        DECLSPEC_XFGVIRT(IEasySFTPDirectory, get_FullPath)
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_FullPath )( 
            IEasySFTPRootDirectory * This,
            /* [retval][out] */ BSTR *pRet);
        
        DECLSPEC_XFGVIRT(IEasySFTPDirectory, get_Url)
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_Url )( 
            IEasySFTPRootDirectory * This,
            /* [retval][out] */ BSTR *pRet);
        
        DECLSPEC_XFGVIRT(IEasySFTPRootDirectory, get_IsUnixServer)
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_IsUnixServer )( 
            IEasySFTPRootDirectory * This,
            /* [retval][out] */ VARIANT_BOOL *pbRet);
        
        DECLSPEC_XFGVIRT(IEasySFTPRootDirectory, get_HostName)
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_HostName )( 
            IEasySFTPRootDirectory * This,
            /* [retval][out] */ BSTR *pRet);
        
        DECLSPEC_XFGVIRT(IEasySFTPRootDirectory, get_Port)
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_Port )( 
            IEasySFTPRootDirectory * This,
            /* [retval][out] */ long *pRet);
        
        DECLSPEC_XFGVIRT(IEasySFTPRootDirectory, get_ConnectionMode)
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_ConnectionMode )( 
            IEasySFTPRootDirectory * This,
            /* [retval][out] */ EasySFTPConnectionMode *pRet);
        
        DECLSPEC_XFGVIRT(IEasySFTPRootDirectory, get_TextMode)
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_TextMode )( 
            IEasySFTPRootDirectory * This,
            /* [retval][out] */ EasySFTPTextMode *pnTextMode);
        
        DECLSPEC_XFGVIRT(IEasySFTPRootDirectory, put_TextMode)
        /* [propput][id] */ HRESULT ( STDMETHODCALLTYPE *put_TextMode )( 
            IEasySFTPRootDirectory * This,
            /* [in] */ EasySFTPTextMode nTextMode);
        
        DECLSPEC_XFGVIRT(IEasySFTPRootDirectory, get_TransferMode)
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_TransferMode )( 
            IEasySFTPRootDirectory * This,
            /* [retval][out] */ EasySFTPTransferMode *pnTransferMode);
        
        DECLSPEC_XFGVIRT(IEasySFTPRootDirectory, put_TransferMode)
        /* [propput][id] */ HRESULT ( STDMETHODCALLTYPE *put_TransferMode )( 
            IEasySFTPRootDirectory * This,
            /* [in] */ EasySFTPTransferMode nTransferMode);
        
        DECLSPEC_XFGVIRT(IEasySFTPRootDirectory, IsTextFile)
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *IsTextFile )( 
            IEasySFTPRootDirectory * This,
            /* [in] */ BSTR lpszFileName,
            /* [retval][out] */ VARIANT_BOOL *pbRet);
        
        DECLSPEC_XFGVIRT(IEasySFTPRootDirectory, Disconnect)
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *Disconnect )( 
            IEasySFTPRootDirectory * This);
        
        DECLSPEC_XFGVIRT(IEasySFTPRootDirectory, get_Connected)
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_Connected )( 
            IEasySFTPRootDirectory * This,
            /* [retval][out] */ VARIANT_BOOL *pbRet);
        
        DECLSPEC_XFGVIRT(IEasySFTPRootDirectory, get_Transferring)
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_Transferring )( 
            IEasySFTPRootDirectory * This,
            /* [retval][out] */ VARIANT_BOOL *pbRet);
        
        END_INTERFACE
    } IEasySFTPRootDirectoryVtbl;

    interface IEasySFTPRootDirectory
    {
        CONST_VTBL struct IEasySFTPRootDirectoryVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEasySFTPRootDirectory_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IEasySFTPRootDirectory_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IEasySFTPRootDirectory_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IEasySFTPRootDirectory_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define IEasySFTPRootDirectory_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define IEasySFTPRootDirectory_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define IEasySFTPRootDirectory_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 


#define IEasySFTPRootDirectory_get_Name(This,pRet)	\
    ( (This)->lpVtbl -> get_Name(This,pRet) ) 

#define IEasySFTPRootDirectory_get_RootDirectory(This,ppRootDirectory)	\
    ( (This)->lpVtbl -> get_RootDirectory(This,ppRootDirectory) ) 

#define IEasySFTPRootDirectory_OpenDirectory(This,File,ppRet)	\
    ( (This)->lpVtbl -> OpenDirectory(This,File,ppRet) ) 

#define IEasySFTPRootDirectory_get_Files(This,ppFiles)	\
    ( (This)->lpVtbl -> get_Files(This,ppFiles) ) 

#define IEasySFTPRootDirectory_OpenTransferDialog(This,hWndOwner)	\
    ( (This)->lpVtbl -> OpenTransferDialog(This,hWndOwner) ) 

#define IEasySFTPRootDirectory_CloseTransferDialog(This)	\
    ( (This)->lpVtbl -> CloseTransferDialog(This) ) 

#define IEasySFTPRootDirectory_OpenFile(This,lpszRelativeFileName,bIsWrite,nTextMode,ppStream)	\
    ( (This)->lpVtbl -> OpenFile(This,lpszRelativeFileName,bIsWrite,nTextMode,ppStream) ) 

#define IEasySFTPRootDirectory_UploadFrom(This,lpszDestinationRelativeName,lpszSourceLocalName,nTextMode)	\
    ( (This)->lpVtbl -> UploadFrom(This,lpszDestinationRelativeName,lpszSourceLocalName,nTextMode) ) 

#define IEasySFTPRootDirectory_UploadFromStream(This,lpszDestinationRelativeFileName,pStream,nTextMode)	\
    ( (This)->lpVtbl -> UploadFromStream(This,lpszDestinationRelativeFileName,pStream,nTextMode) ) 

#define IEasySFTPRootDirectory_UploadFromDataObject(This,pObject,nTextMode)	\
    ( (This)->lpVtbl -> UploadFromDataObject(This,pObject,nTextMode) ) 

#define IEasySFTPRootDirectory_DownloadTo(This,File,lpszTargetLocalName,nTextMode)	\
    ( (This)->lpVtbl -> DownloadTo(This,File,lpszTargetLocalName,nTextMode) ) 

#define IEasySFTPRootDirectory_DownloadToStream(This,File,pStream,nTextMode)	\
    ( (This)->lpVtbl -> DownloadToStream(This,File,pStream,nTextMode) ) 

#define IEasySFTPRootDirectory_UploadFiles(This,LocalFiles,nTextMode)	\
    ( (This)->lpVtbl -> UploadFiles(This,LocalFiles,nTextMode) ) 

#define IEasySFTPRootDirectory_DownloadFiles(This,RemoteFiles,bstrDestinationDirectory,nTextMode)	\
    ( (This)->lpVtbl -> DownloadFiles(This,RemoteFiles,bstrDestinationDirectory,nTextMode) ) 

#define IEasySFTPRootDirectory_Move(This,File,lpszTargetName)	\
    ( (This)->lpVtbl -> Move(This,File,lpszTargetName) ) 

#define IEasySFTPRootDirectory_Remove(This,File)	\
    ( (This)->lpVtbl -> Remove(This,File) ) 

#define IEasySFTPRootDirectory_UpdateFileTime(This,File,modifyTime,createTime,accessTime)	\
    ( (This)->lpVtbl -> UpdateFileTime(This,File,modifyTime,createTime,accessTime) ) 

#define IEasySFTPRootDirectory_UpdateAttributes(This,File,attr)	\
    ( (This)->lpVtbl -> UpdateAttributes(This,File,attr) ) 

#define IEasySFTPRootDirectory_CreateShortcut(This,LinkName,TargetName)	\
    ( (This)->lpVtbl -> CreateShortcut(This,LinkName,TargetName) ) 

#define IEasySFTPRootDirectory_get_FullPath(This,pRet)	\
    ( (This)->lpVtbl -> get_FullPath(This,pRet) ) 

#define IEasySFTPRootDirectory_get_Url(This,pRet)	\
    ( (This)->lpVtbl -> get_Url(This,pRet) ) 


#define IEasySFTPRootDirectory_get_IsUnixServer(This,pbRet)	\
    ( (This)->lpVtbl -> get_IsUnixServer(This,pbRet) ) 

#define IEasySFTPRootDirectory_get_HostName(This,pRet)	\
    ( (This)->lpVtbl -> get_HostName(This,pRet) ) 

#define IEasySFTPRootDirectory_get_Port(This,pRet)	\
    ( (This)->lpVtbl -> get_Port(This,pRet) ) 

#define IEasySFTPRootDirectory_get_ConnectionMode(This,pRet)	\
    ( (This)->lpVtbl -> get_ConnectionMode(This,pRet) ) 

#define IEasySFTPRootDirectory_get_TextMode(This,pnTextMode)	\
    ( (This)->lpVtbl -> get_TextMode(This,pnTextMode) ) 

#define IEasySFTPRootDirectory_put_TextMode(This,nTextMode)	\
    ( (This)->lpVtbl -> put_TextMode(This,nTextMode) ) 

#define IEasySFTPRootDirectory_get_TransferMode(This,pnTransferMode)	\
    ( (This)->lpVtbl -> get_TransferMode(This,pnTransferMode) ) 

#define IEasySFTPRootDirectory_put_TransferMode(This,nTransferMode)	\
    ( (This)->lpVtbl -> put_TransferMode(This,nTransferMode) ) 

#define IEasySFTPRootDirectory_IsTextFile(This,lpszFileName,pbRet)	\
    ( (This)->lpVtbl -> IsTextFile(This,lpszFileName,pbRet) ) 

#define IEasySFTPRootDirectory_Disconnect(This)	\
    ( (This)->lpVtbl -> Disconnect(This) ) 

#define IEasySFTPRootDirectory_get_Connected(This,pbRet)	\
    ( (This)->lpVtbl -> get_Connected(This,pbRet) ) 

#define IEasySFTPRootDirectory_get_Transferring(This,pbRet)	\
    ( (This)->lpVtbl -> get_Transferring(This,pbRet) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IEasySFTPRootDirectory_INTERFACE_DEFINED__ */


#ifndef __IEasySFTPAuthentication_INTERFACE_DEFINED__
#define __IEasySFTPAuthentication_INTERFACE_DEFINED__

/* interface IEasySFTPAuthentication */
/* [object][dual][unique][uuid] */ 


EXTERN_C const IID IID_IEasySFTPAuthentication;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("AD29C042-B9E3-4650-9DF6-D7DA5B8D0199")
    IEasySFTPAuthentication : public IDispatch
    {
    public:
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_UserName( 
            /* [retval][out] */ BSTR *pRet) = 0;
        
        virtual /* [propput][id] */ HRESULT STDMETHODCALLTYPE put_UserName( 
            /* [in] */ BSTR Name) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_Password( 
            /* [retval][out] */ BSTR *pRet) = 0;
        
        virtual /* [propput][id] */ HRESULT STDMETHODCALLTYPE put_Password( 
            /* [in] */ BSTR Password) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_PublicKeyFileName( 
            /* [retval][out] */ BSTR *pRet) = 0;
        
        virtual /* [propput][id] */ HRESULT STDMETHODCALLTYPE put_PublicKeyFileName( 
            /* [in] */ BSTR FileName) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_Type( 
            /* [retval][out] */ EasySFTPAuthenticationMode *pMode) = 0;
        
        virtual /* [propput][id] */ HRESULT STDMETHODCALLTYPE put_Type( 
            /* [in] */ EasySFTPAuthenticationMode mode) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_AuthSession( 
            /* [retval][out] */ LONG_PTR *pRet) = 0;
        
        virtual /* [propput][id] */ HRESULT STDMETHODCALLTYPE put_AuthSession( 
            /* [in] */ LONG_PTR session) = 0;
        
    };
    
    
#else 	/* C style interface */

    typedef struct IEasySFTPAuthenticationVtbl
    {
        BEGIN_INTERFACE
        
        DECLSPEC_XFGVIRT(IUnknown, QueryInterface)
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEasySFTPAuthentication * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        DECLSPEC_XFGVIRT(IUnknown, AddRef)
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEasySFTPAuthentication * This);
        
        DECLSPEC_XFGVIRT(IUnknown, Release)
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEasySFTPAuthentication * This);
        
        DECLSPEC_XFGVIRT(IDispatch, GetTypeInfoCount)
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IEasySFTPAuthentication * This,
            /* [out] */ UINT *pctinfo);
        
        DECLSPEC_XFGVIRT(IDispatch, GetTypeInfo)
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IEasySFTPAuthentication * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        DECLSPEC_XFGVIRT(IDispatch, GetIDsOfNames)
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IEasySFTPAuthentication * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        DECLSPEC_XFGVIRT(IDispatch, Invoke)
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IEasySFTPAuthentication * This,
            /* [annotation][in] */ 
            _In_  DISPID dispIdMember,
            /* [annotation][in] */ 
            _In_  REFIID riid,
            /* [annotation][in] */ 
            _In_  LCID lcid,
            /* [annotation][in] */ 
            _In_  WORD wFlags,
            /* [annotation][out][in] */ 
            _In_  DISPPARAMS *pDispParams,
            /* [annotation][out] */ 
            _Out_opt_  VARIANT *pVarResult,
            /* [annotation][out] */ 
            _Out_opt_  EXCEPINFO *pExcepInfo,
            /* [annotation][out] */ 
            _Out_opt_  UINT *puArgErr);
        
        DECLSPEC_XFGVIRT(IEasySFTPAuthentication, get_UserName)
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_UserName )( 
            IEasySFTPAuthentication * This,
            /* [retval][out] */ BSTR *pRet);
        
        DECLSPEC_XFGVIRT(IEasySFTPAuthentication, put_UserName)
        /* [propput][id] */ HRESULT ( STDMETHODCALLTYPE *put_UserName )( 
            IEasySFTPAuthentication * This,
            /* [in] */ BSTR Name);
        
        DECLSPEC_XFGVIRT(IEasySFTPAuthentication, get_Password)
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_Password )( 
            IEasySFTPAuthentication * This,
            /* [retval][out] */ BSTR *pRet);
        
        DECLSPEC_XFGVIRT(IEasySFTPAuthentication, put_Password)
        /* [propput][id] */ HRESULT ( STDMETHODCALLTYPE *put_Password )( 
            IEasySFTPAuthentication * This,
            /* [in] */ BSTR Password);
        
        DECLSPEC_XFGVIRT(IEasySFTPAuthentication, get_PublicKeyFileName)
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_PublicKeyFileName )( 
            IEasySFTPAuthentication * This,
            /* [retval][out] */ BSTR *pRet);
        
        DECLSPEC_XFGVIRT(IEasySFTPAuthentication, put_PublicKeyFileName)
        /* [propput][id] */ HRESULT ( STDMETHODCALLTYPE *put_PublicKeyFileName )( 
            IEasySFTPAuthentication * This,
            /* [in] */ BSTR FileName);
        
        DECLSPEC_XFGVIRT(IEasySFTPAuthentication, get_Type)
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_Type )( 
            IEasySFTPAuthentication * This,
            /* [retval][out] */ EasySFTPAuthenticationMode *pMode);
        
        DECLSPEC_XFGVIRT(IEasySFTPAuthentication, put_Type)
        /* [propput][id] */ HRESULT ( STDMETHODCALLTYPE *put_Type )( 
            IEasySFTPAuthentication * This,
            /* [in] */ EasySFTPAuthenticationMode mode);
        
        DECLSPEC_XFGVIRT(IEasySFTPAuthentication, get_AuthSession)
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_AuthSession )( 
            IEasySFTPAuthentication * This,
            /* [retval][out] */ LONG_PTR *pRet);
        
        DECLSPEC_XFGVIRT(IEasySFTPAuthentication, put_AuthSession)
        /* [propput][id] */ HRESULT ( STDMETHODCALLTYPE *put_AuthSession )( 
            IEasySFTPAuthentication * This,
            /* [in] */ LONG_PTR session);
        
        END_INTERFACE
    } IEasySFTPAuthenticationVtbl;

    interface IEasySFTPAuthentication
    {
        CONST_VTBL struct IEasySFTPAuthenticationVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEasySFTPAuthentication_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IEasySFTPAuthentication_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IEasySFTPAuthentication_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IEasySFTPAuthentication_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define IEasySFTPAuthentication_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define IEasySFTPAuthentication_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define IEasySFTPAuthentication_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 


#define IEasySFTPAuthentication_get_UserName(This,pRet)	\
    ( (This)->lpVtbl -> get_UserName(This,pRet) ) 

#define IEasySFTPAuthentication_put_UserName(This,Name)	\
    ( (This)->lpVtbl -> put_UserName(This,Name) ) 

#define IEasySFTPAuthentication_get_Password(This,pRet)	\
    ( (This)->lpVtbl -> get_Password(This,pRet) ) 

#define IEasySFTPAuthentication_put_Password(This,Password)	\
    ( (This)->lpVtbl -> put_Password(This,Password) ) 

#define IEasySFTPAuthentication_get_PublicKeyFileName(This,pRet)	\
    ( (This)->lpVtbl -> get_PublicKeyFileName(This,pRet) ) 

#define IEasySFTPAuthentication_put_PublicKeyFileName(This,FileName)	\
    ( (This)->lpVtbl -> put_PublicKeyFileName(This,FileName) ) 

#define IEasySFTPAuthentication_get_Type(This,pMode)	\
    ( (This)->lpVtbl -> get_Type(This,pMode) ) 

#define IEasySFTPAuthentication_put_Type(This,mode)	\
    ( (This)->lpVtbl -> put_Type(This,mode) ) 

#define IEasySFTPAuthentication_get_AuthSession(This,pRet)	\
    ( (This)->lpVtbl -> get_AuthSession(This,pRet) ) 

#define IEasySFTPAuthentication_put_AuthSession(This,session)	\
    ( (This)->lpVtbl -> put_AuthSession(This,session) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IEasySFTPAuthentication_INTERFACE_DEFINED__ */


#ifndef __IEasySFTPRoot_INTERFACE_DEFINED__
#define __IEasySFTPRoot_INTERFACE_DEFINED__

/* interface IEasySFTPRoot */
/* [object][dual][unique][uuid] */ 

#undef NoConversion

EXTERN_C const IID IID_IEasySFTPRoot;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("AD29C042-B9E3-4644-9DF6-D7DA5B8D0199")
    IEasySFTPRoot : public IDispatch
    {
    public:
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_Version( 
            /* [retval][out] */ BSTR *pRet) = 0;
        
        virtual /* [hidden][id] */ HRESULT STDMETHODCALLTYPE _Connect( 
            /* [in] */ VARIANT_BOOL bSFTP,
            /* [in] */ LONG_PTR hWnd,
            /* [in] */ LONG_PTR pvReserved,
            /* [in] */ BSTR lpszHostName,
            /* [in] */ long nPort,
            /* [retval][out] */ IEasySFTPRootDirectory **ppFolder) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE QuickConnectDialog( 
            /* [in] */ LONG_PTR hWnd,
            /* [retval][out] */ IEasySFTPRootDirectory **ppFolder) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE GetDependencyLibraryInfo( 
            /* [retval][out] */ BSTR *poutLibraryInfo) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE Connect( 
            /* [in] */ EasySFTPConnectionMode ConnectionMode,
            /* [in] */ LONG_PTR hWnd,
            /* [in] */ IEasySFTPAuthentication *pAuth,
            /* [in] */ BSTR lpszHostName,
            /* [defaultvalue][in] */ long nPort,
            /* [defaultvalue][in] */ VARIANT_BOOL bIgnoreFingerprint,
            /* [retval][out] */ IEasySFTPRootDirectory **ppFolder) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE CreateAuthentication( 
            /* [in] */ EasySFTPAuthenticationMode mode,
            /* [in] */ BSTR bstrUserName,
            /* [retval][out] */ IEasySFTPAuthentication **ppAuth) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_HostSettings( 
            /* [retval][out] */ IEasySFTPHostSettingList **ppList) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE CreateHostSetting( 
            /* [retval][out] */ IEasySFTPHostSetting **ppRet) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE ConnectFromSetting( 
            /* [in] */ LONG_PTR hWnd,
            /* [in] */ IEasySFTPHostSetting *pSetting,
            /* [defaultvalue][in] */ VARIANT_BOOL bIgnoreFingerprint,
            /* [retval][out] */ IEasySFTPDirectory **ppFolder) = 0;
        
    };
    
    
#else 	/* C style interface */

    typedef struct IEasySFTPRootVtbl
    {
        BEGIN_INTERFACE
        
        DECLSPEC_XFGVIRT(IUnknown, QueryInterface)
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEasySFTPRoot * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        DECLSPEC_XFGVIRT(IUnknown, AddRef)
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEasySFTPRoot * This);
        
        DECLSPEC_XFGVIRT(IUnknown, Release)
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEasySFTPRoot * This);
        
        DECLSPEC_XFGVIRT(IDispatch, GetTypeInfoCount)
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IEasySFTPRoot * This,
            /* [out] */ UINT *pctinfo);
        
        DECLSPEC_XFGVIRT(IDispatch, GetTypeInfo)
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IEasySFTPRoot * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        DECLSPEC_XFGVIRT(IDispatch, GetIDsOfNames)
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IEasySFTPRoot * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        DECLSPEC_XFGVIRT(IDispatch, Invoke)
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IEasySFTPRoot * This,
            /* [annotation][in] */ 
            _In_  DISPID dispIdMember,
            /* [annotation][in] */ 
            _In_  REFIID riid,
            /* [annotation][in] */ 
            _In_  LCID lcid,
            /* [annotation][in] */ 
            _In_  WORD wFlags,
            /* [annotation][out][in] */ 
            _In_  DISPPARAMS *pDispParams,
            /* [annotation][out] */ 
            _Out_opt_  VARIANT *pVarResult,
            /* [annotation][out] */ 
            _Out_opt_  EXCEPINFO *pExcepInfo,
            /* [annotation][out] */ 
            _Out_opt_  UINT *puArgErr);
        
        DECLSPEC_XFGVIRT(IEasySFTPRoot, get_Version)
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_Version )( 
            IEasySFTPRoot * This,
            /* [retval][out] */ BSTR *pRet);
        
        DECLSPEC_XFGVIRT(IEasySFTPRoot, _Connect)
        /* [hidden][id] */ HRESULT ( STDMETHODCALLTYPE *_Connect )( 
            IEasySFTPRoot * This,
            /* [in] */ VARIANT_BOOL bSFTP,
            /* [in] */ LONG_PTR hWnd,
            /* [in] */ LONG_PTR pvReserved,
            /* [in] */ BSTR lpszHostName,
            /* [in] */ long nPort,
            /* [retval][out] */ IEasySFTPRootDirectory **ppFolder);
        
        DECLSPEC_XFGVIRT(IEasySFTPRoot, QuickConnectDialog)
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *QuickConnectDialog )( 
            IEasySFTPRoot * This,
            /* [in] */ LONG_PTR hWnd,
            /* [retval][out] */ IEasySFTPRootDirectory **ppFolder);
        
        DECLSPEC_XFGVIRT(IEasySFTPRoot, GetDependencyLibraryInfo)
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *GetDependencyLibraryInfo )( 
            IEasySFTPRoot * This,
            /* [retval][out] */ BSTR *poutLibraryInfo);
        
        DECLSPEC_XFGVIRT(IEasySFTPRoot, Connect)
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *Connect )( 
            IEasySFTPRoot * This,
            /* [in] */ EasySFTPConnectionMode ConnectionMode,
            /* [in] */ LONG_PTR hWnd,
            /* [in] */ IEasySFTPAuthentication *pAuth,
            /* [in] */ BSTR lpszHostName,
            /* [defaultvalue][in] */ long nPort,
            /* [defaultvalue][in] */ VARIANT_BOOL bIgnoreFingerprint,
            /* [retval][out] */ IEasySFTPRootDirectory **ppFolder);
        
        DECLSPEC_XFGVIRT(IEasySFTPRoot, CreateAuthentication)
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *CreateAuthentication )( 
            IEasySFTPRoot * This,
            /* [in] */ EasySFTPAuthenticationMode mode,
            /* [in] */ BSTR bstrUserName,
            /* [retval][out] */ IEasySFTPAuthentication **ppAuth);
        
        DECLSPEC_XFGVIRT(IEasySFTPRoot, get_HostSettings)
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_HostSettings )( 
            IEasySFTPRoot * This,
            /* [retval][out] */ IEasySFTPHostSettingList **ppList);
        
        DECLSPEC_XFGVIRT(IEasySFTPRoot, CreateHostSetting)
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *CreateHostSetting )( 
            IEasySFTPRoot * This,
            /* [retval][out] */ IEasySFTPHostSetting **ppRet);
        
        DECLSPEC_XFGVIRT(IEasySFTPRoot, ConnectFromSetting)
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *ConnectFromSetting )( 
            IEasySFTPRoot * This,
            /* [in] */ LONG_PTR hWnd,
            /* [in] */ IEasySFTPHostSetting *pSetting,
            /* [defaultvalue][in] */ VARIANT_BOOL bIgnoreFingerprint,
            /* [retval][out] */ IEasySFTPDirectory **ppFolder);
        
        END_INTERFACE
    } IEasySFTPRootVtbl;

    interface IEasySFTPRoot
    {
        CONST_VTBL struct IEasySFTPRootVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEasySFTPRoot_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IEasySFTPRoot_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IEasySFTPRoot_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IEasySFTPRoot_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define IEasySFTPRoot_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define IEasySFTPRoot_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define IEasySFTPRoot_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 


#define IEasySFTPRoot_get_Version(This,pRet)	\
    ( (This)->lpVtbl -> get_Version(This,pRet) ) 

#define IEasySFTPRoot__Connect(This,bSFTP,hWnd,pvReserved,lpszHostName,nPort,ppFolder)	\
    ( (This)->lpVtbl -> _Connect(This,bSFTP,hWnd,pvReserved,lpszHostName,nPort,ppFolder) ) 

#define IEasySFTPRoot_QuickConnectDialog(This,hWnd,ppFolder)	\
    ( (This)->lpVtbl -> QuickConnectDialog(This,hWnd,ppFolder) ) 

#define IEasySFTPRoot_GetDependencyLibraryInfo(This,poutLibraryInfo)	\
    ( (This)->lpVtbl -> GetDependencyLibraryInfo(This,poutLibraryInfo) ) 

#define IEasySFTPRoot_Connect(This,ConnectionMode,hWnd,pAuth,lpszHostName,nPort,bIgnoreFingerprint,ppFolder)	\
    ( (This)->lpVtbl -> Connect(This,ConnectionMode,hWnd,pAuth,lpszHostName,nPort,bIgnoreFingerprint,ppFolder) ) 

#define IEasySFTPRoot_CreateAuthentication(This,mode,bstrUserName,ppAuth)	\
    ( (This)->lpVtbl -> CreateAuthentication(This,mode,bstrUserName,ppAuth) ) 

#define IEasySFTPRoot_get_HostSettings(This,ppList)	\
    ( (This)->lpVtbl -> get_HostSettings(This,ppList) ) 

#define IEasySFTPRoot_CreateHostSetting(This,ppRet)	\
    ( (This)->lpVtbl -> CreateHostSetting(This,ppRet) ) 

#define IEasySFTPRoot_ConnectFromSetting(This,hWnd,pSetting,bIgnoreFingerprint,ppFolder)	\
    ( (This)->lpVtbl -> ConnectFromSetting(This,hWnd,pSetting,bIgnoreFingerprint,ppFolder) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IEasySFTPRoot_INTERFACE_DEFINED__ */


#ifndef __IEasySFTPStringList_INTERFACE_DEFINED__
#define __IEasySFTPStringList_INTERFACE_DEFINED__

/* interface IEasySFTPStringList */
/* [object][dual][unique][uuid] */ 


EXTERN_C const IID IID_IEasySFTPStringList;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("AD29C042-B9E3-4654-9DF6-D7DA5B8D0199")
    IEasySFTPStringList : public IDispatch
    {
    public:
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_Item( 
            /* [in] */ long Index,
            /* [retval][out] */ BSTR *pString) = 0;
        
        virtual /* [propput][id] */ HRESULT STDMETHODCALLTYPE put_Item( 
            /* [in] */ long Index,
            /* [in] */ BSTR String) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_Count( 
            /* [retval][out] */ long *pRet) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE Add( 
            /* [in] */ BSTR String,
            /* [retval][out] */ long *pRet) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE Remove( 
            /* [in] */ long Index) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE FindIndex( 
            /* [in] */ BSTR String,
            /* [retval][out] */ long *pRet) = 0;
        
        virtual /* [hidden][propget][id] */ HRESULT STDMETHODCALLTYPE get__NewEnum( 
            /* [retval][out] */ IUnknown **ppRet) = 0;
        
    };
    
    
#else 	/* C style interface */

    typedef struct IEasySFTPStringListVtbl
    {
        BEGIN_INTERFACE
        
        DECLSPEC_XFGVIRT(IUnknown, QueryInterface)
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEasySFTPStringList * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        DECLSPEC_XFGVIRT(IUnknown, AddRef)
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEasySFTPStringList * This);
        
        DECLSPEC_XFGVIRT(IUnknown, Release)
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEasySFTPStringList * This);
        
        DECLSPEC_XFGVIRT(IDispatch, GetTypeInfoCount)
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IEasySFTPStringList * This,
            /* [out] */ UINT *pctinfo);
        
        DECLSPEC_XFGVIRT(IDispatch, GetTypeInfo)
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IEasySFTPStringList * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        DECLSPEC_XFGVIRT(IDispatch, GetIDsOfNames)
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IEasySFTPStringList * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        DECLSPEC_XFGVIRT(IDispatch, Invoke)
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IEasySFTPStringList * This,
            /* [annotation][in] */ 
            _In_  DISPID dispIdMember,
            /* [annotation][in] */ 
            _In_  REFIID riid,
            /* [annotation][in] */ 
            _In_  LCID lcid,
            /* [annotation][in] */ 
            _In_  WORD wFlags,
            /* [annotation][out][in] */ 
            _In_  DISPPARAMS *pDispParams,
            /* [annotation][out] */ 
            _Out_opt_  VARIANT *pVarResult,
            /* [annotation][out] */ 
            _Out_opt_  EXCEPINFO *pExcepInfo,
            /* [annotation][out] */ 
            _Out_opt_  UINT *puArgErr);
        
        DECLSPEC_XFGVIRT(IEasySFTPStringList, get_Item)
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_Item )( 
            IEasySFTPStringList * This,
            /* [in] */ long Index,
            /* [retval][out] */ BSTR *pString);
        
        DECLSPEC_XFGVIRT(IEasySFTPStringList, put_Item)
        /* [propput][id] */ HRESULT ( STDMETHODCALLTYPE *put_Item )( 
            IEasySFTPStringList * This,
            /* [in] */ long Index,
            /* [in] */ BSTR String);
        
        DECLSPEC_XFGVIRT(IEasySFTPStringList, get_Count)
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            IEasySFTPStringList * This,
            /* [retval][out] */ long *pRet);
        
        DECLSPEC_XFGVIRT(IEasySFTPStringList, Add)
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *Add )( 
            IEasySFTPStringList * This,
            /* [in] */ BSTR String,
            /* [retval][out] */ long *pRet);
        
        DECLSPEC_XFGVIRT(IEasySFTPStringList, Remove)
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *Remove )( 
            IEasySFTPStringList * This,
            /* [in] */ long Index);
        
        DECLSPEC_XFGVIRT(IEasySFTPStringList, FindIndex)
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *FindIndex )( 
            IEasySFTPStringList * This,
            /* [in] */ BSTR String,
            /* [retval][out] */ long *pRet);
        
        DECLSPEC_XFGVIRT(IEasySFTPStringList, get__NewEnum)
        /* [hidden][propget][id] */ HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            IEasySFTPStringList * This,
            /* [retval][out] */ IUnknown **ppRet);
        
        END_INTERFACE
    } IEasySFTPStringListVtbl;

    interface IEasySFTPStringList
    {
        CONST_VTBL struct IEasySFTPStringListVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEasySFTPStringList_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IEasySFTPStringList_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IEasySFTPStringList_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IEasySFTPStringList_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define IEasySFTPStringList_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define IEasySFTPStringList_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define IEasySFTPStringList_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 


#define IEasySFTPStringList_get_Item(This,Index,pString)	\
    ( (This)->lpVtbl -> get_Item(This,Index,pString) ) 

#define IEasySFTPStringList_put_Item(This,Index,String)	\
    ( (This)->lpVtbl -> put_Item(This,Index,String) ) 

#define IEasySFTPStringList_get_Count(This,pRet)	\
    ( (This)->lpVtbl -> get_Count(This,pRet) ) 

#define IEasySFTPStringList_Add(This,String,pRet)	\
    ( (This)->lpVtbl -> Add(This,String,pRet) ) 

#define IEasySFTPStringList_Remove(This,Index)	\
    ( (This)->lpVtbl -> Remove(This,Index) ) 

#define IEasySFTPStringList_FindIndex(This,String,pRet)	\
    ( (This)->lpVtbl -> FindIndex(This,String,pRet) ) 

#define IEasySFTPStringList_get__NewEnum(This,ppRet)	\
    ( (This)->lpVtbl -> get__NewEnum(This,ppRet) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IEasySFTPStringList_INTERFACE_DEFINED__ */


#ifndef __IEasySFTPHostSetting_INTERFACE_DEFINED__
#define __IEasySFTPHostSetting_INTERFACE_DEFINED__

/* interface IEasySFTPHostSetting */
/* [object][dual][unique][uuid] */ 


EXTERN_C const IID IID_IEasySFTPHostSetting;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("AD29C042-B9E3-4652-9DF6-D7DA5B8D0199")
    IEasySFTPHostSetting : public IDispatch
    {
    public:
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_Name( 
            /* [retval][out] */ BSTR *pRet) = 0;
        
        virtual /* [propput][id] */ HRESULT STDMETHODCALLTYPE put_Name( 
            /* [in] */ BSTR Value) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_HostName( 
            /* [retval][out] */ BSTR *pRet) = 0;
        
        virtual /* [propput][id] */ HRESULT STDMETHODCALLTYPE put_HostName( 
            /* [in] */ BSTR Value) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_ConnectionMode( 
            /* [retval][out] */ EasySFTPConnectionMode *pRet) = 0;
        
        virtual /* [propput][id] */ HRESULT STDMETHODCALLTYPE put_ConnectionMode( 
            /* [in] */ EasySFTPConnectionMode Value) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_Port( 
            /* [retval][out] */ long *pRet) = 0;
        
        virtual /* [propput][id] */ HRESULT STDMETHODCALLTYPE put_Port( 
            /* [in] */ long Value) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_InitLocalPath( 
            /* [retval][out] */ BSTR *pRet) = 0;
        
        virtual /* [propput][id] */ HRESULT STDMETHODCALLTYPE put_InitLocalPath( 
            /* [in] */ BSTR Value) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_InitServerPath( 
            /* [retval][out] */ BSTR *pRet) = 0;
        
        virtual /* [propput][id] */ HRESULT STDMETHODCALLTYPE put_InitServerPath( 
            /* [in] */ BSTR Value) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_TextMode( 
            /* [retval][out] */ EasySFTPTextMode *pRet) = 0;
        
        virtual /* [propput][id] */ HRESULT STDMETHODCALLTYPE put_TextMode( 
            /* [in] */ EasySFTPTextMode Value) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_ServerCharset( 
            /* [retval][out] */ EasySFTPServerCharset *pRet) = 0;
        
        virtual /* [propput][id] */ HRESULT STDMETHODCALLTYPE put_ServerCharset( 
            /* [in] */ EasySFTPServerCharset Value) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_TransferMode( 
            /* [retval][out] */ EasySFTPTransferMode *pRet) = 0;
        
        virtual /* [propput][id] */ HRESULT STDMETHODCALLTYPE put_TransferMode( 
            /* [in] */ EasySFTPTransferMode Value) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_TextFileType( 
            /* [retval][out] */ IEasySFTPStringList **ppList) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_UseSystemTextFileType( 
            /* [retval][out] */ VARIANT_BOOL *pRet) = 0;
        
        virtual /* [propput][id] */ HRESULT STDMETHODCALLTYPE put_UseSystemTextFileType( 
            /* [in] */ VARIANT_BOOL Value) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_AdjustRecvModifyTime( 
            /* [retval][out] */ VARIANT_BOOL *pRet) = 0;
        
        virtual /* [propput][id] */ HRESULT STDMETHODCALLTYPE put_AdjustRecvModifyTime( 
            /* [in] */ VARIANT_BOOL Value) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_AdjustSendModifyTime( 
            /* [retval][out] */ VARIANT_BOOL *pRet) = 0;
        
        virtual /* [propput][id] */ HRESULT STDMETHODCALLTYPE put_AdjustSendModifyTime( 
            /* [in] */ VARIANT_BOOL Value) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_UseThumbnailPreview( 
            /* [retval][out] */ VARIANT_BOOL *pRet) = 0;
        
        virtual /* [propput][id] */ HRESULT STDMETHODCALLTYPE put_UseThumbnailPreview( 
            /* [in] */ VARIANT_BOOL Value) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_ChmodCommand( 
            /* [retval][out] */ BSTR *pRet) = 0;
        
        virtual /* [propput][id] */ HRESULT STDMETHODCALLTYPE put_ChmodCommand( 
            /* [in] */ BSTR Value) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE CopyFrom( 
            /* [in] */ IEasySFTPHostSetting *pSetting) = 0;
        
    };
    
    
#else 	/* C style interface */

    typedef struct IEasySFTPHostSettingVtbl
    {
        BEGIN_INTERFACE
        
        DECLSPEC_XFGVIRT(IUnknown, QueryInterface)
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEasySFTPHostSetting * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        DECLSPEC_XFGVIRT(IUnknown, AddRef)
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEasySFTPHostSetting * This);
        
        DECLSPEC_XFGVIRT(IUnknown, Release)
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEasySFTPHostSetting * This);
        
        DECLSPEC_XFGVIRT(IDispatch, GetTypeInfoCount)
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IEasySFTPHostSetting * This,
            /* [out] */ UINT *pctinfo);
        
        DECLSPEC_XFGVIRT(IDispatch, GetTypeInfo)
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IEasySFTPHostSetting * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        DECLSPEC_XFGVIRT(IDispatch, GetIDsOfNames)
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IEasySFTPHostSetting * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        DECLSPEC_XFGVIRT(IDispatch, Invoke)
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IEasySFTPHostSetting * This,
            /* [annotation][in] */ 
            _In_  DISPID dispIdMember,
            /* [annotation][in] */ 
            _In_  REFIID riid,
            /* [annotation][in] */ 
            _In_  LCID lcid,
            /* [annotation][in] */ 
            _In_  WORD wFlags,
            /* [annotation][out][in] */ 
            _In_  DISPPARAMS *pDispParams,
            /* [annotation][out] */ 
            _Out_opt_  VARIANT *pVarResult,
            /* [annotation][out] */ 
            _Out_opt_  EXCEPINFO *pExcepInfo,
            /* [annotation][out] */ 
            _Out_opt_  UINT *puArgErr);
        
        DECLSPEC_XFGVIRT(IEasySFTPHostSetting, get_Name)
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            IEasySFTPHostSetting * This,
            /* [retval][out] */ BSTR *pRet);
        
        DECLSPEC_XFGVIRT(IEasySFTPHostSetting, put_Name)
        /* [propput][id] */ HRESULT ( STDMETHODCALLTYPE *put_Name )( 
            IEasySFTPHostSetting * This,
            /* [in] */ BSTR Value);
        
        DECLSPEC_XFGVIRT(IEasySFTPHostSetting, get_HostName)
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_HostName )( 
            IEasySFTPHostSetting * This,
            /* [retval][out] */ BSTR *pRet);
        
        DECLSPEC_XFGVIRT(IEasySFTPHostSetting, put_HostName)
        /* [propput][id] */ HRESULT ( STDMETHODCALLTYPE *put_HostName )( 
            IEasySFTPHostSetting * This,
            /* [in] */ BSTR Value);
        
        DECLSPEC_XFGVIRT(IEasySFTPHostSetting, get_ConnectionMode)
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_ConnectionMode )( 
            IEasySFTPHostSetting * This,
            /* [retval][out] */ EasySFTPConnectionMode *pRet);
        
        DECLSPEC_XFGVIRT(IEasySFTPHostSetting, put_ConnectionMode)
        /* [propput][id] */ HRESULT ( STDMETHODCALLTYPE *put_ConnectionMode )( 
            IEasySFTPHostSetting * This,
            /* [in] */ EasySFTPConnectionMode Value);
        
        DECLSPEC_XFGVIRT(IEasySFTPHostSetting, get_Port)
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_Port )( 
            IEasySFTPHostSetting * This,
            /* [retval][out] */ long *pRet);
        
        DECLSPEC_XFGVIRT(IEasySFTPHostSetting, put_Port)
        /* [propput][id] */ HRESULT ( STDMETHODCALLTYPE *put_Port )( 
            IEasySFTPHostSetting * This,
            /* [in] */ long Value);
        
        DECLSPEC_XFGVIRT(IEasySFTPHostSetting, get_InitLocalPath)
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_InitLocalPath )( 
            IEasySFTPHostSetting * This,
            /* [retval][out] */ BSTR *pRet);
        
        DECLSPEC_XFGVIRT(IEasySFTPHostSetting, put_InitLocalPath)
        /* [propput][id] */ HRESULT ( STDMETHODCALLTYPE *put_InitLocalPath )( 
            IEasySFTPHostSetting * This,
            /* [in] */ BSTR Value);
        
        DECLSPEC_XFGVIRT(IEasySFTPHostSetting, get_InitServerPath)
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_InitServerPath )( 
            IEasySFTPHostSetting * This,
            /* [retval][out] */ BSTR *pRet);
        
        DECLSPEC_XFGVIRT(IEasySFTPHostSetting, put_InitServerPath)
        /* [propput][id] */ HRESULT ( STDMETHODCALLTYPE *put_InitServerPath )( 
            IEasySFTPHostSetting * This,
            /* [in] */ BSTR Value);
        
        DECLSPEC_XFGVIRT(IEasySFTPHostSetting, get_TextMode)
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_TextMode )( 
            IEasySFTPHostSetting * This,
            /* [retval][out] */ EasySFTPTextMode *pRet);
        
        DECLSPEC_XFGVIRT(IEasySFTPHostSetting, put_TextMode)
        /* [propput][id] */ HRESULT ( STDMETHODCALLTYPE *put_TextMode )( 
            IEasySFTPHostSetting * This,
            /* [in] */ EasySFTPTextMode Value);
        
        DECLSPEC_XFGVIRT(IEasySFTPHostSetting, get_ServerCharset)
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_ServerCharset )( 
            IEasySFTPHostSetting * This,
            /* [retval][out] */ EasySFTPServerCharset *pRet);
        
        DECLSPEC_XFGVIRT(IEasySFTPHostSetting, put_ServerCharset)
        /* [propput][id] */ HRESULT ( STDMETHODCALLTYPE *put_ServerCharset )( 
            IEasySFTPHostSetting * This,
            /* [in] */ EasySFTPServerCharset Value);
        
        DECLSPEC_XFGVIRT(IEasySFTPHostSetting, get_TransferMode)
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_TransferMode )( 
            IEasySFTPHostSetting * This,
            /* [retval][out] */ EasySFTPTransferMode *pRet);
        
        DECLSPEC_XFGVIRT(IEasySFTPHostSetting, put_TransferMode)
        /* [propput][id] */ HRESULT ( STDMETHODCALLTYPE *put_TransferMode )( 
            IEasySFTPHostSetting * This,
            /* [in] */ EasySFTPTransferMode Value);
        
        DECLSPEC_XFGVIRT(IEasySFTPHostSetting, get_TextFileType)
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_TextFileType )( 
            IEasySFTPHostSetting * This,
            /* [retval][out] */ IEasySFTPStringList **ppList);
        
        DECLSPEC_XFGVIRT(IEasySFTPHostSetting, get_UseSystemTextFileType)
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_UseSystemTextFileType )( 
            IEasySFTPHostSetting * This,
            /* [retval][out] */ VARIANT_BOOL *pRet);
        
        DECLSPEC_XFGVIRT(IEasySFTPHostSetting, put_UseSystemTextFileType)
        /* [propput][id] */ HRESULT ( STDMETHODCALLTYPE *put_UseSystemTextFileType )( 
            IEasySFTPHostSetting * This,
            /* [in] */ VARIANT_BOOL Value);
        
        DECLSPEC_XFGVIRT(IEasySFTPHostSetting, get_AdjustRecvModifyTime)
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_AdjustRecvModifyTime )( 
            IEasySFTPHostSetting * This,
            /* [retval][out] */ VARIANT_BOOL *pRet);
        
        DECLSPEC_XFGVIRT(IEasySFTPHostSetting, put_AdjustRecvModifyTime)
        /* [propput][id] */ HRESULT ( STDMETHODCALLTYPE *put_AdjustRecvModifyTime )( 
            IEasySFTPHostSetting * This,
            /* [in] */ VARIANT_BOOL Value);
        
        DECLSPEC_XFGVIRT(IEasySFTPHostSetting, get_AdjustSendModifyTime)
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_AdjustSendModifyTime )( 
            IEasySFTPHostSetting * This,
            /* [retval][out] */ VARIANT_BOOL *pRet);
        
        DECLSPEC_XFGVIRT(IEasySFTPHostSetting, put_AdjustSendModifyTime)
        /* [propput][id] */ HRESULT ( STDMETHODCALLTYPE *put_AdjustSendModifyTime )( 
            IEasySFTPHostSetting * This,
            /* [in] */ VARIANT_BOOL Value);
        
        DECLSPEC_XFGVIRT(IEasySFTPHostSetting, get_UseThumbnailPreview)
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_UseThumbnailPreview )( 
            IEasySFTPHostSetting * This,
            /* [retval][out] */ VARIANT_BOOL *pRet);
        
        DECLSPEC_XFGVIRT(IEasySFTPHostSetting, put_UseThumbnailPreview)
        /* [propput][id] */ HRESULT ( STDMETHODCALLTYPE *put_UseThumbnailPreview )( 
            IEasySFTPHostSetting * This,
            /* [in] */ VARIANT_BOOL Value);
        
        DECLSPEC_XFGVIRT(IEasySFTPHostSetting, get_ChmodCommand)
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_ChmodCommand )( 
            IEasySFTPHostSetting * This,
            /* [retval][out] */ BSTR *pRet);
        
        DECLSPEC_XFGVIRT(IEasySFTPHostSetting, put_ChmodCommand)
        /* [propput][id] */ HRESULT ( STDMETHODCALLTYPE *put_ChmodCommand )( 
            IEasySFTPHostSetting * This,
            /* [in] */ BSTR Value);
        
        DECLSPEC_XFGVIRT(IEasySFTPHostSetting, CopyFrom)
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *CopyFrom )( 
            IEasySFTPHostSetting * This,
            /* [in] */ IEasySFTPHostSetting *pSetting);
        
        END_INTERFACE
    } IEasySFTPHostSettingVtbl;

    interface IEasySFTPHostSetting
    {
        CONST_VTBL struct IEasySFTPHostSettingVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEasySFTPHostSetting_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IEasySFTPHostSetting_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IEasySFTPHostSetting_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IEasySFTPHostSetting_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define IEasySFTPHostSetting_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define IEasySFTPHostSetting_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define IEasySFTPHostSetting_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 


#define IEasySFTPHostSetting_get_Name(This,pRet)	\
    ( (This)->lpVtbl -> get_Name(This,pRet) ) 

#define IEasySFTPHostSetting_put_Name(This,Value)	\
    ( (This)->lpVtbl -> put_Name(This,Value) ) 

#define IEasySFTPHostSetting_get_HostName(This,pRet)	\
    ( (This)->lpVtbl -> get_HostName(This,pRet) ) 

#define IEasySFTPHostSetting_put_HostName(This,Value)	\
    ( (This)->lpVtbl -> put_HostName(This,Value) ) 

#define IEasySFTPHostSetting_get_ConnectionMode(This,pRet)	\
    ( (This)->lpVtbl -> get_ConnectionMode(This,pRet) ) 

#define IEasySFTPHostSetting_put_ConnectionMode(This,Value)	\
    ( (This)->lpVtbl -> put_ConnectionMode(This,Value) ) 

#define IEasySFTPHostSetting_get_Port(This,pRet)	\
    ( (This)->lpVtbl -> get_Port(This,pRet) ) 

#define IEasySFTPHostSetting_put_Port(This,Value)	\
    ( (This)->lpVtbl -> put_Port(This,Value) ) 

#define IEasySFTPHostSetting_get_InitLocalPath(This,pRet)	\
    ( (This)->lpVtbl -> get_InitLocalPath(This,pRet) ) 

#define IEasySFTPHostSetting_put_InitLocalPath(This,Value)	\
    ( (This)->lpVtbl -> put_InitLocalPath(This,Value) ) 

#define IEasySFTPHostSetting_get_InitServerPath(This,pRet)	\
    ( (This)->lpVtbl -> get_InitServerPath(This,pRet) ) 

#define IEasySFTPHostSetting_put_InitServerPath(This,Value)	\
    ( (This)->lpVtbl -> put_InitServerPath(This,Value) ) 

#define IEasySFTPHostSetting_get_TextMode(This,pRet)	\
    ( (This)->lpVtbl -> get_TextMode(This,pRet) ) 

#define IEasySFTPHostSetting_put_TextMode(This,Value)	\
    ( (This)->lpVtbl -> put_TextMode(This,Value) ) 

#define IEasySFTPHostSetting_get_ServerCharset(This,pRet)	\
    ( (This)->lpVtbl -> get_ServerCharset(This,pRet) ) 

#define IEasySFTPHostSetting_put_ServerCharset(This,Value)	\
    ( (This)->lpVtbl -> put_ServerCharset(This,Value) ) 

#define IEasySFTPHostSetting_get_TransferMode(This,pRet)	\
    ( (This)->lpVtbl -> get_TransferMode(This,pRet) ) 

#define IEasySFTPHostSetting_put_TransferMode(This,Value)	\
    ( (This)->lpVtbl -> put_TransferMode(This,Value) ) 

#define IEasySFTPHostSetting_get_TextFileType(This,ppList)	\
    ( (This)->lpVtbl -> get_TextFileType(This,ppList) ) 

#define IEasySFTPHostSetting_get_UseSystemTextFileType(This,pRet)	\
    ( (This)->lpVtbl -> get_UseSystemTextFileType(This,pRet) ) 

#define IEasySFTPHostSetting_put_UseSystemTextFileType(This,Value)	\
    ( (This)->lpVtbl -> put_UseSystemTextFileType(This,Value) ) 

#define IEasySFTPHostSetting_get_AdjustRecvModifyTime(This,pRet)	\
    ( (This)->lpVtbl -> get_AdjustRecvModifyTime(This,pRet) ) 

#define IEasySFTPHostSetting_put_AdjustRecvModifyTime(This,Value)	\
    ( (This)->lpVtbl -> put_AdjustRecvModifyTime(This,Value) ) 

#define IEasySFTPHostSetting_get_AdjustSendModifyTime(This,pRet)	\
    ( (This)->lpVtbl -> get_AdjustSendModifyTime(This,pRet) ) 

#define IEasySFTPHostSetting_put_AdjustSendModifyTime(This,Value)	\
    ( (This)->lpVtbl -> put_AdjustSendModifyTime(This,Value) ) 

#define IEasySFTPHostSetting_get_UseThumbnailPreview(This,pRet)	\
    ( (This)->lpVtbl -> get_UseThumbnailPreview(This,pRet) ) 

#define IEasySFTPHostSetting_put_UseThumbnailPreview(This,Value)	\
    ( (This)->lpVtbl -> put_UseThumbnailPreview(This,Value) ) 

#define IEasySFTPHostSetting_get_ChmodCommand(This,pRet)	\
    ( (This)->lpVtbl -> get_ChmodCommand(This,pRet) ) 

#define IEasySFTPHostSetting_put_ChmodCommand(This,Value)	\
    ( (This)->lpVtbl -> put_ChmodCommand(This,Value) ) 

#define IEasySFTPHostSetting_CopyFrom(This,pSetting)	\
    ( (This)->lpVtbl -> CopyFrom(This,pSetting) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IEasySFTPHostSetting_INTERFACE_DEFINED__ */


#ifndef __IEasySFTPHostSettingList_INTERFACE_DEFINED__
#define __IEasySFTPHostSettingList_INTERFACE_DEFINED__

/* interface IEasySFTPHostSettingList */
/* [object][dual][unique][uuid] */ 


EXTERN_C const IID IID_IEasySFTPHostSettingList;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("AD29C042-B9E3-4656-9DF6-D7DA5B8D0199")
    IEasySFTPHostSettingList : public IDispatch
    {
    public:
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_Item( 
            /* [in] */ long Index,
            /* [retval][out] */ IEasySFTPHostSetting **ppRet) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_Count( 
            /* [retval][out] */ long *pRet) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE Add( 
            /* [in] */ IEasySFTPHostSetting *Setting,
            /* [retval][out] */ long *pRet) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE Remove( 
            /* [in] */ long Index) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE FindHost( 
            /* [in] */ BSTR HostName,
            /* [in] */ EasySFTPConnectionMode Mode,
            /* [defaultvalue][in] */ long Port,
            /* [retval][out] */ IEasySFTPHostSetting **ppRet) = 0;
        
        virtual /* [hidden][propget][id] */ HRESULT STDMETHODCALLTYPE get__NewEnum( 
            /* [retval][out] */ IUnknown **ppRet) = 0;
        
    };
    
    
#else 	/* C style interface */

    typedef struct IEasySFTPHostSettingListVtbl
    {
        BEGIN_INTERFACE
        
        DECLSPEC_XFGVIRT(IUnknown, QueryInterface)
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEasySFTPHostSettingList * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        DECLSPEC_XFGVIRT(IUnknown, AddRef)
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEasySFTPHostSettingList * This);
        
        DECLSPEC_XFGVIRT(IUnknown, Release)
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEasySFTPHostSettingList * This);
        
        DECLSPEC_XFGVIRT(IDispatch, GetTypeInfoCount)
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IEasySFTPHostSettingList * This,
            /* [out] */ UINT *pctinfo);
        
        DECLSPEC_XFGVIRT(IDispatch, GetTypeInfo)
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IEasySFTPHostSettingList * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        DECLSPEC_XFGVIRT(IDispatch, GetIDsOfNames)
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IEasySFTPHostSettingList * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        DECLSPEC_XFGVIRT(IDispatch, Invoke)
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IEasySFTPHostSettingList * This,
            /* [annotation][in] */ 
            _In_  DISPID dispIdMember,
            /* [annotation][in] */ 
            _In_  REFIID riid,
            /* [annotation][in] */ 
            _In_  LCID lcid,
            /* [annotation][in] */ 
            _In_  WORD wFlags,
            /* [annotation][out][in] */ 
            _In_  DISPPARAMS *pDispParams,
            /* [annotation][out] */ 
            _Out_opt_  VARIANT *pVarResult,
            /* [annotation][out] */ 
            _Out_opt_  EXCEPINFO *pExcepInfo,
            /* [annotation][out] */ 
            _Out_opt_  UINT *puArgErr);
        
        DECLSPEC_XFGVIRT(IEasySFTPHostSettingList, get_Item)
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_Item )( 
            IEasySFTPHostSettingList * This,
            /* [in] */ long Index,
            /* [retval][out] */ IEasySFTPHostSetting **ppRet);
        
        DECLSPEC_XFGVIRT(IEasySFTPHostSettingList, get_Count)
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            IEasySFTPHostSettingList * This,
            /* [retval][out] */ long *pRet);
        
        DECLSPEC_XFGVIRT(IEasySFTPHostSettingList, Add)
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *Add )( 
            IEasySFTPHostSettingList * This,
            /* [in] */ IEasySFTPHostSetting *Setting,
            /* [retval][out] */ long *pRet);
        
        DECLSPEC_XFGVIRT(IEasySFTPHostSettingList, Remove)
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *Remove )( 
            IEasySFTPHostSettingList * This,
            /* [in] */ long Index);
        
        DECLSPEC_XFGVIRT(IEasySFTPHostSettingList, FindHost)
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *FindHost )( 
            IEasySFTPHostSettingList * This,
            /* [in] */ BSTR HostName,
            /* [in] */ EasySFTPConnectionMode Mode,
            /* [defaultvalue][in] */ long Port,
            /* [retval][out] */ IEasySFTPHostSetting **ppRet);
        
        DECLSPEC_XFGVIRT(IEasySFTPHostSettingList, get__NewEnum)
        /* [hidden][propget][id] */ HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            IEasySFTPHostSettingList * This,
            /* [retval][out] */ IUnknown **ppRet);
        
        END_INTERFACE
    } IEasySFTPHostSettingListVtbl;

    interface IEasySFTPHostSettingList
    {
        CONST_VTBL struct IEasySFTPHostSettingListVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEasySFTPHostSettingList_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IEasySFTPHostSettingList_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IEasySFTPHostSettingList_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IEasySFTPHostSettingList_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define IEasySFTPHostSettingList_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define IEasySFTPHostSettingList_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define IEasySFTPHostSettingList_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 


#define IEasySFTPHostSettingList_get_Item(This,Index,ppRet)	\
    ( (This)->lpVtbl -> get_Item(This,Index,ppRet) ) 

#define IEasySFTPHostSettingList_get_Count(This,pRet)	\
    ( (This)->lpVtbl -> get_Count(This,pRet) ) 

#define IEasySFTPHostSettingList_Add(This,Setting,pRet)	\
    ( (This)->lpVtbl -> Add(This,Setting,pRet) ) 

#define IEasySFTPHostSettingList_Remove(This,Index)	\
    ( (This)->lpVtbl -> Remove(This,Index) ) 

#define IEasySFTPHostSettingList_FindHost(This,HostName,Mode,Port,ppRet)	\
    ( (This)->lpVtbl -> FindHost(This,HostName,Mode,Port,ppRet) ) 

#define IEasySFTPHostSettingList_get__NewEnum(This,ppRet)	\
    ( (This)->lpVtbl -> get__NewEnum(This,ppRet) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IEasySFTPHostSettingList_INTERFACE_DEFINED__ */


EXTERN_C const CLSID CLSID_EasySFTPRoot;

#ifdef __cplusplus

class DECLSPEC_UUID("AD29C042-B9E3-4740-9DF6-D7DA5B8D0199")
EasySFTPRoot;
#endif

EXTERN_C const CLSID CLSID_EasySFTPFile;

#ifdef __cplusplus

class DECLSPEC_UUID("AD29C042-B9E3-4742-9DF6-D7DA5B8D0199")
EasySFTPFile;
#endif

EXTERN_C const CLSID CLSID_EasySFTPFiles;

#ifdef __cplusplus

class DECLSPEC_UUID("AD29C042-B9E3-4744-9DF6-D7DA5B8D0199")
EasySFTPFiles;
#endif

EXTERN_C const CLSID CLSID_EasySFTPStream;

#ifdef __cplusplus

class DECLSPEC_UUID("AD29C042-B9E3-4746-9DF6-D7DA5B8D0199")
EasySFTPStream;
#endif

EXTERN_C const CLSID CLSID_EasySFTPDirectory;

#ifdef __cplusplus

class DECLSPEC_UUID("AD29C042-B9E3-4748-9DF6-D7DA5B8D0199")
EasySFTPDirectory;
#endif

EXTERN_C const CLSID CLSID_EasySFTPRootDirectory;

#ifdef __cplusplus

class DECLSPEC_UUID("AD29C042-B9E3-474a-9DF6-D7DA5B8D0199")
EasySFTPRootDirectory;
#endif

EXTERN_C const CLSID CLSID_EasySFTPAuthentication;

#ifdef __cplusplus

class DECLSPEC_UUID("AD29C042-B9E3-474c-9DF6-D7DA5B8D0199")
EasySFTPAuthentication;
#endif

EXTERN_C const CLSID CLSID_EasySFTPStringList;

#ifdef __cplusplus

class DECLSPEC_UUID("AD29C042-B9E3-474e-9DF6-D7DA5B8D0199")
EasySFTPStringList;
#endif

EXTERN_C const CLSID CLSID_EasySFTPHostSetting;

#ifdef __cplusplus

class DECLSPEC_UUID("AD29C042-B9E3-4750-9DF6-D7DA5B8D0199")
EasySFTPHostSetting;
#endif

EXTERN_C const CLSID CLSID_EasySFTPHostSettingList;

#ifdef __cplusplus

class DECLSPEC_UUID("AD29C042-B9E3-4752-9DF6-D7DA5B8D0199")
EasySFTPHostSettingList;
#endif
#endif /* __EasySFTP_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


