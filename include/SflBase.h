/////////////////////////////////////////////////////////////////////////////
//
//  Windows Service Framework Library (SFL)
//
//  Version 2.0
//
//
//  SflBase.h
//
//  Last update: Nov 05 2007
//
//
//
//  Copyright (c) 2004,2007 Igor Vartanov
//
//  mailto: igor_vartanov@mail.ru
//
/////////////////////////////////////////////////////////////////////////////

#ifndef IV_SFLBASE_H
#define IV_SFLBASE_H

#include "crtdbg.h"
#include "tchar.h"
#include "malloc.h"

#define _SFL_VER 0x0200

#ifdef _MSC_VER
#pragma comment( lib, "advapi32.lib" )
#endif

#if defined(_MSC_VER) && (_MSC_VER < 1300)
#pragma warning(push)
#pragma warning(disable : 4100 4127 4511 4512 4710)
#endif

#ifndef SFLASSERT
#define SFLASSERT(x)   _ASSERTE(x)
#endif

#ifndef ARRAYSIZE
#define ARRAYSIZE(x)  (sizeof(x)/sizeof(x[0]))
#endif

namespace SFL {
	class CServiceRoot;

	enum
	{
		SFL_SERVICESTYLE_NT4 = 0,
		SFL_SERVICESTYLE_NT5
	};
	template <int> class X;	
	typedef X<SFL_SERVICESTYLE_NT4> XNT4; 
	typedef X<SFL_SERVICESTYLE_NT5> XNT5;

	/////////////////////////////////////////////////////////////////////////////
	//
	//  structure CServiceInfo
	//
	//  Version 2.0 (Jul 15 2007)
	//
	//
	/////////////////////////////////////////////////////////////////////////////

	struct CServiceInfo
	{
		LPCTSTR                 m_pszServiceName;
		LPSERVICE_MAIN_FUNCTION m_pfnServiceMain;
		DWORD                   m_style;
	};

	/////////////////////////////////////////////////////////////////////////////
	//
	//  class CServiceProxyT<TService,t_szServiceName>
	//
	//  Version 2.0 (Sep 28 2007)
	//
	//
	/////////////////////////////////////////////////////////////////////////////
	template<class TService, DWORD t_serviceId>
	class CServiceProxyT: protected TService
	{
		typedef CServiceProxyT<TService, t_serviceId> CServiceProxyClass;
		template <class T> void SetHandler(T*);
		template <> void SetHandler<XNT4>(XNT4*) { m_pfnHandler   = _Handler;   }
		template <> void SetHandler<XNT5>(XNT5*) { m_pfnHandlerEx = _HandlerEx; }

	protected:
		CServiceProxyT()
		{
			m_pThis = this;			
			SetHandler((X<serviceStyle>*)NULL);
		}

	public:
		static CServiceRoot* Construct(TService* pDummy = NULL)
		{
			static TCHAR szServiceName[256] = {0};
			BOOL res = ::LoadString(::GetModuleHandle(NULL), t_serviceId, szServiceName, ARRAYSIZE(szServiceName));
			SFLASSERT(res);
			return Construct2(pDummy, szServiceName);
		}

		static CServiceRoot* Construct2(TService* pDummy = NULL, LPCTSTR pszName = NULL)
		{
			pDummy; // to prevent warning C4100
			static CServiceProxyClass theService;
			static CServiceInfo si = { pszName, theService._ServiceMain, theService.serviceStyle };
			theService.m_pServiceInfo = &si;
			return static_cast<CServiceRoot*>(&theService);
		}

	protected:
		static CServiceProxyClass* m_pThis;

		static void WINAPI _ServiceMain(DWORD dwArgc, LPTSTR* lpszArgv)
		{
			SFLASSERT(m_pThis || !"SFLServiceProxyT<TService>::_ServiceMain");
			m_pThis->ServiceMain(dwArgc, lpszArgv);
		}

		static void WINAPI _Handler(DWORD dwCtrl)
		{
			SFLASSERT(m_pThis || !"SFLServiceProxyT<TService>::_Handler");
			m_pThis->Handler(dwCtrl);
		}

		static DWORD WINAPI _HandlerEx(DWORD dwCtrl, DWORD dwEventType, LPVOID lpEventData, LPVOID lpContext)
		{
			SFLASSERT(m_pThis || !"SFLServiceProxyT<TService>::_HandlerEx");
			return m_pThis->HandlerEx(dwCtrl, dwEventType, lpEventData, lpContext);
		}
	};

	template<class TService, DWORD t_szServiceId>
		CServiceProxyT<TService, t_szServiceId>* CServiceProxyT<TService, t_szServiceId>::m_pThis = NULL;


    /////////////////////////////////////////////////////////////////////////////
    //
    //  class CServiceStatusObject
    //
    //  Version 2.0 (Jul 01 2006)
    //
    //
    /////////////////////////////////////////////////////////////////////////////

    class CServiceStatusObject: public SERVICE_STATUS
    {
    private:
        SERVICE_STATUS_HANDLE m_handle;

    public:
        CServiceStatusObject(): m_handle(NULL) 
		{
			ZeroMemory(static_cast<LPSERVICE_STATUS>(this), sizeof(SERVICE_STATUS));
		}

        operator LPSERVICE_STATUS()
        {
            return static_cast<LPSERVICE_STATUS>(this);
        }

        SERVICE_STATUS& operator=( SERVICE_STATUS& status )
        {
            *static_cast<LPSERVICE_STATUS>(this) = status;
            return *this;
        }

        SERVICE_STATUS_HANDLE GetHandle() const
        {
            return m_handle;
        }

        SERVICE_STATUS_HANDLE SetHandle( SERVICE_STATUS_HANDLE handle )
        {
            m_handle = handle;
            return m_handle;
        }
    };


    /////////////////////////////////////////////////////////////////////////////
    //
    //  class CServiceRoot
    //
    //  Version 2.0 (Jul 15 2007)
    //
    //
    /////////////////////////////////////////////////////////////////////////////

    class CServiceRoot
    {
    protected:
        CServiceInfo*  m_pServiceInfo;
        CServiceStatusObject m_status;
        
	protected:
		CServiceRoot(): m_pServiceInfo(NULL) {}

    private:
        CServiceRoot( const CServiceRoot& ) {}
        CServiceRoot& operator=( const CServiceRoot& ) {}

    public:
        virtual ~CServiceRoot() {}

        DWORD GetCurrentState() const
        {
            return m_status.dwCurrentState;
        }

        LPCTSTR GetServiceName() const
        {
            return m_pServiceInfo->m_pszServiceName;
        }

		LPSERVICE_MAIN_FUNCTION GetServiceMain() const
		{
			return m_pServiceInfo->m_pfnServiceMain;
		}

		DWORD GetServiceStyle() const
		{
			return m_pServiceInfo->m_style;
		}

        void SetServiceType( DWORD dwType )
        {
			if(0 == m_status.dwServiceType)  // can be set only once
				m_status.dwServiceType = dwType;
        }

        DWORD GetControlsAccepted() const
        {
            return m_status.dwControlsAccepted;
        }

	protected:  
        void SetControlsAccepted( DWORD dwAccept )
        {
            m_status.dwControlsAccepted = dwAccept;
        }

        BOOL SetServiceStatus( LPSERVICE_STATUS pStatus = NULL )
        {
            SFLASSERT( m_status.GetHandle() || !"SFL: Handler not registered yet" );

            if( pStatus )
                m_status = *pStatus;

            switch( m_status.dwCurrentState )
            {
            case SERVICE_START_PENDING:
            case SERVICE_STOP_PENDING:
            case SERVICE_CONTINUE_PENDING:
            case SERVICE_PAUSE_PENDING:
                break;

            default:
                m_status.dwCheckPoint = m_status.dwWaitHint = 0;
                break;
            }

            if( ( ERROR_SUCCESS != m_status.dwServiceSpecificExitCode ) &&
                ( ERROR_SERVICE_SPECIFIC_ERROR != m_status.dwWin32ExitCode ) )
                m_status.dwWin32ExitCode = ERROR_SERVICE_SPECIFIC_ERROR;

            return ::SetServiceStatus( m_status.GetHandle(), m_status );
        }

        BOOL SetServiceStatus( DWORD dwState, DWORD dwWin32ExitCode = ERROR_SUCCESS )
        {
            m_status.dwCurrentState  = dwState;
            m_status.dwWin32ExitCode = dwWin32ExitCode;
            m_status.dwServiceSpecificExitCode = 0;

            return SetServiceStatus();
        }

        BOOL SetServiceStatusSpecific( DWORD dwState, DWORD dwSpecificExitCode )
        {
            if( ERROR_SUCCESS == dwSpecificExitCode )
                return SetServiceStatus( dwState );

            m_status.dwServiceSpecificExitCode = dwSpecificExitCode;        
            return SetServiceStatus( dwState, ERROR_SERVICE_SPECIFIC_ERROR );
        }

        BOOL CheckPoint( DWORD dwWaitHint, DWORD dwCheckPoint = DWORD(-1) )
        {
            switch( m_status.dwCurrentState )
            {
            case SERVICE_START_PENDING:
            case SERVICE_STOP_PENDING:
            case SERVICE_CONTINUE_PENDING:
            case SERVICE_PAUSE_PENDING:
                {
                    if( DWORD(-1) == dwCheckPoint )
                        m_status.dwCheckPoint++;
                    else
                        m_status.dwCheckPoint = dwCheckPoint;

                    m_status.dwWaitHint = dwWaitHint;
                }
                break;

            default:
                SFLASSERT( !"SFL: Checking the point in nonpending state" );
                m_status.dwCheckPoint = m_status.dwWaitHint = 0;
                break;
            }

            return SetServiceStatus();
        }
    };


    /////////////////////////////////////////////////////////////////////////////
    //
    //  class CServiceBaseT<T,t_dwControlAccepted>
    //
    //  Version 2.0 (Nov 05 2007)
    //
    //
    /////////////////////////////////////////////////////////////////////////////

    template<class TService, DWORD t_dwControlsAccepted>
    class CServiceBaseT: public CServiceRoot
    {
    protected:
        typedef CServiceBaseT<TService, t_dwControlsAccepted> CServiceBaseClass;

        CServiceBaseT(){}

		union
		{
			LPHANDLER_FUNCTION    m_pfnHandler;
			LPHANDLER_FUNCTION_EX m_pfnHandlerEx;
		};

		template <class T> SERVICE_STATUS_HANDLE RegisterHandler(T*);
		template <> SERVICE_STATUS_HANDLE RegisterHandler<XNT4>(XNT4*)
		{
			return ::RegisterServiceCtrlHandler( GetServiceName(), m_pfnHandler );
		}

		template <> SERVICE_STATUS_HANDLE RegisterHandler<XNT5>(XNT5*)
		{
			TService* pThis = static_cast<TService*>(this);
			SFLASSERT( pThis || "CServiceBaseClass::ServiceMain" );
			return ::RegisterServiceCtrlHandlerEx( GetServiceName(), m_pfnHandlerEx, pThis->GetServiceContext() );
		}

        void ServiceMain( DWORD dwArgc, LPTSTR* lpszArgv )
        {
            TService* pThis = static_cast<TService*>(this);
            SFLASSERT( pThis || "CServiceBaseClass::ServiceMain" );

            BOOL  bRes;
            DWORD dwSpecific = 0;

            SetControlsAccepted( t_dwControlsAccepted );

			m_status.SetHandle( pThis->RegisterHandler( (X<TService::serviceStyle>*)NULL ) );
            SFLASSERT( m_status.GetHandle() || !"SFL: Handler registration failed" );

            SetServiceStatus( SERVICE_START_PENDING );

            bRes = pThis->InitInstance( dwArgc, lpszArgv, dwSpecific );
            if( !bRes )
            {
                SetServiceStatusSpecific( SERVICE_STOPPED, dwSpecific );
                return;
            }

            bRes = SetServiceStatus( SERVICE_RUNNING );
            SFLASSERT( bRes || !"SFL: Failed to set RUNNING status" );
        }

        BOOL InitInstance( DWORD dwArgc, LPTSTR* lpszArgv, DWORD& dwSpecific )
        {
            dwArgc; lpszArgv; dwSpecific; // prevent warning C4100

            return TRUE;
        }

		LPVOID GetServiceContext() 
		{ 
			return NULL; 
		}
    };


    /////////////////////////////////////////////////////////////////////////////
    //
    //  class CServiceAppT<T>
    //
    //  Version 2.0 (Jul 15 2007)
    //
    //
    /////////////////////////////////////////////////////////////////////////////
    template <class T>
    class CServiceAppT
    {
    private:
        size_t m_nServiceCount;
        CServiceRoot** m_pServices;

    protected:
        typedef CServiceAppT<T> CAppBaseClass;

        CServiceAppT(): m_nServiceCount(0), m_pServices(NULL)
        {
        }

        BOOL Run( LPSERVICE_TABLE_ENTRY pTable )
        {
            T* pThis = static_cast<T*>(this);

            if( !pThis->InitApp() )
                return FALSE;

            BOOL bRes = ::StartServiceCtrlDispatcher( pTable );
            SFLASSERT( bRes || !"SFL: StartServiceCtrlDispatcher() failed" );

            pThis->ExitApp();

            return bRes;
        }

    public:
        int GetServiceCount() const
        {
            return m_nServiceCount;
        }

        CServiceInfo** GetServices() const
        {
            return m_pServices;
        }

        BOOL InitApp()
        {
            return TRUE;
        }

        void ExitApp()
        {
        }

        BOOL PreMain( int argc, LPTSTR* argv )
        {
            argc; argv; // to prevent warning C4100
            return TRUE;
        }

        void PostMain()
        {
        }

        int Main( int argc, LPTSTR* argv, CServiceRoot** pMap, size_t nTableSize )
        {
            argc; argv; // prevent warning C4100
            LPSERVICE_TABLE_ENTRY pTable = NULL;
            BOOL bRes       = 0;

            m_pServices     = pMap;
            m_nServiceCount = nTableSize - 1;  // because nTableSize includes final NULL item

            size_t nSize = sizeof(SERVICE_TABLE_ENTRY) * nTableSize;
            __try
            {
                pTable = (LPSERVICE_TABLE_ENTRY)_alloca( nSize );
                ::ZeroMemory( pTable, nSize );

                // fill service table with service map objects data
                for( size_t i = 0; i < m_nServiceCount; i++ )
                {
                    SFLASSERT( pMap[i]->GetServiceName() || !"SFL: Service NULL name not allowed" );
                    pTable[i].lpServiceName = (LPTSTR)pMap[i]->GetServiceName();
					pTable[i].lpServiceProc = pMap[i]->GetServiceMain();
                    pMap[i]->SetServiceType( (m_nServiceCount > 1)? SERVICE_WIN32_SHARE_PROCESS : SERVICE_WIN32_OWN_PROCESS );
                }

                T* pThis = static_cast<T*>(this);
                bRes = pThis->Run( pTable );
            }
            __except( STATUS_STACK_OVERFLOW == GetExceptionCode() )
            {
                SFLASSERT(!"SFL: Stack overflow in CServiceAppT<>::Main()");
            }
            return !bRes;
        }
    };

} // namespace SFL

#if defined(_MSC_VER) && (_MSC_VER < 1300)
#pragma warning(pop)
#endif

#ifdef SFL_USE_NAMESPACE_EXPLICITLY
#define SFL_NS  SFL::
#else
#define SFL_NS
using namespace SFL;
#endif // SFL_USE_NAMESPACE_EXPLICITLY

/////////////////////////////////////////////////////////////////////////////
//
//  The set of macros
//
//  Version 2.0 (Sep 28 2007)
//
/////////////////////////////////////////////////////////////////////////////

//////////// SERVICE APPLICATION  ///////////////////////////////////////////
//
#define SFL_DECLARE_SERVICEAPP(T) class T; namespace SFL{ const T* SflGetServiceApp(void); }

//////////// SERVICE MAP ////////////////////////////////////////////////////
//
#define SFL_BEGIN_SERVICE_MAP(T)                                            \
namespace SFL{ const T* SflGetServiceApp(void) { static T theApp; return &theApp; } } \
    int _tmain( int argc, LPTSTR* argv )                                    \
    {                                                                       \
        T* pApp = const_cast<T*>( SFL_NS SflGetServiceApp() );              \
        SFL::CServiceRoot* pServiceMap[] = {                                \

#define SFL_SERVICE_ENTRY( TService, idRes )                                \
	SFL::CServiceProxyT< TService, idRes >::Construct( (TService*)NULL ),   \

#define SFL_SERVICE_ENTRY2( TService, id, name )                            \
    SFL::CServiceProxyT< TService, id >::Construct2( (TService*)NULL, TEXT( name ) ), \

#define SFL_END_SERVICE_MAP()                                               \
			NULL                                                            \
        };                                                                  \
        int retMain = -1;                                                   \
        if( pApp->PreMain( argc, argv ) )                                   \
            retMain = pApp->Main( argc, argv, pServiceMap, ARRAYSIZE(pServiceMap) ); \
        pApp->PostMain();                                                   \
        return retMain;                                                     \
        }

//////////// CONTROL HANDLING MAP ///////////////////////////////////////////
//
#define SFL_BEGIN_CONTROL_MAP(T)                                            \
public:	                                                                    \
    enum { serviceStyle = SFL_NS SFL_SERVICESTYLE_NT4 };                    \
protected:                                                                  \
    void Handler( DWORD dwControl )                                         \
    {                                                                       \
        BOOL    bHandled       = FALSE;                                     \
        DWORD   dwState        = m_status.dwCurrentState;                   \
        DWORD   dwWin32Err     = 0;                                         \
        DWORD   dwSpecificErr  = 0;                                         \
typedef T THandlerClass;                                                    \
typedef DWORD (T::*t_handler)(DWORD&, DWORD&, BOOL&);                       \
typedef DWORD (T::*t_handler_range)(DWORD, DWORD&, DWORD&, BOOL&);          \
        do {                                                                \

#define SFL_END_CONTROL_MAP()                                               \
        } while(0);                                                         \
        if( bHandled )                                                      \
        {   m_status.dwCurrentState = dwState;                              \
            m_status.dwWin32ExitCode = dwWin32Err;                          \
            m_status.dwServiceSpecificExitCode = dwSpecificErr;             \
            SetServiceStatus();                                             \
        }                                                                   \
    }                                                                       \

#define SFL_HANDLE_CONTROL( code, handler )                                 \
        if( code == dwControl )                                             \
        {   t_handler pfnHandler = static_cast<t_handler>(&THandlerClass:: handler); \
            dwState = (this->*pfnHandler)( dwWin32Err, dwSpecificErr,       \
                                           bHandled ); break;               \
        }

#define SFL_HANDLE_CONTROL_RANGE( codeMin, codeMax, handler )               \
        if( (codeMin <= dwControl) && (codeMax >= dwControl ) )             \
        {   t_handler_range pfnHandler =                                    \
                static_cast<t_handler_range>(&THandlerClass:: handler);     \
            dwState = (this->*pfnHandler)( dwControl, dwWin32Err,           \
                                           dwSpecificErr, bHandled );       \
            break;                                                          \
        }

#define SFL_HANDLE_CONTROL_CONTINUE()    SFL_HANDLE_CONTROL( SERVICE_CONTROL_CONTINUE, OnContinue )
#define SFL_HANDLE_CONTROL_PAUSE()       SFL_HANDLE_CONTROL( SERVICE_CONTROL_PAUSE, OnPause )
#define SFL_HANDLE_CONTROL_STOP()        SFL_HANDLE_CONTROL( SERVICE_CONTROL_STOP, OnStop )
#define SFL_HANDLE_CONTROL_SHUTDOWN()    SFL_HANDLE_CONTROL( SERVICE_CONTROL_SHUTDOWN, OnShutdown )
#define SFL_HANDLE_CONTROL_INTERROGATE() SFL_HANDLE_CONTROL( SERVICE_CONTROL_INTERROGATE, OnInterrogate )


//////////// EXTENDED CONTROL HANDLING MAP //////////////////////////////////
//
#define SFL_BEGIN_CONTROL_MAP_EX(T)                                         \
public:	                                                                    \
    enum { serviceStyle = SFL_NS SFL_SERVICESTYLE_NT5 };                    \
protected:                                                                  \
    DWORD HandlerEx( DWORD  dwControl,   DWORD  dwEventType,                \
                     LPVOID lpEventData, LPVOID lpContext )                 \
    {                                                                       \
        BOOL    bHandled       = FALSE;                                     \
        DWORD   dwRet          = NO_ERROR;                                  \
        DWORD   dwState        = m_status.dwCurrentState;                   \
        DWORD   dwWin32Err     = 0;                                         \
        DWORD   dwSpecificErr  = 0;                                         \
typedef T THandlerClass;                                                    \
typedef DWORD (T::*t_handler)(DWORD&, DWORD&, BOOL&);                       \
typedef DWORD (T::*t_handler_range)(DWORD, DWORD&, DWORD&, BOOL&);          \
typedef DWORD (T::*t_handler_ex)(DWORD&, DWORD&, DWORD&, BOOL&, DWORD, LPVOID, LPVOID); \
typedef DWORD (T::*t_handler_range_ex)(DWORD, DWORD&, DWORD&, DWORD&, BOOL&, DWORD, LPVOID, LPVOID); \
	    dwEventType; lpEventData; lpContext;  /* preventing 4100 */         \
        do {                                                                \

#define SFL_END_CONTROL_MAP_EX()                                            \
        } while(0);                                                         \
        if( bHandled )                                                      \
        {   m_status.dwCurrentState = dwState;                              \
            m_status.dwWin32ExitCode = dwWin32Err;                          \
            m_status.dwServiceSpecificExitCode = dwSpecificErr;             \
            SetServiceStatus();                                             \
            return dwRet; }                                                 \
        return ERROR_CALL_NOT_IMPLEMENTED;                                  \
    }                                                                       \

#define SFL_HANDLE_CONTROL_EX( code, handler )                              \
        if( code == dwControl )                                             \
        {   t_handler_ex pfnHandler = static_cast<t_handler_ex>(&THandlerClass:: handler); \
            dwRet = (this->*pfnHandler)( dwState, dwWin32Err, dwSpecificErr,\
                                         bHandled, dwEventType,             \
                                         lpEventData, lpContext ); break; }

#define SFL_HANDLE_CONTROL_RANGE_EX( codeMin, codeMax, handler )            \
        if( (codeMin <= dwControl) && (codeMax >= dwControl ) )             \
        {   t_handler_range_ex pfnHandler =                                 \
                static_cast<t_handler_range_ex>(&THandlerClass:: handler);  \
            dwRet = (this->*pfnHandler)( dwControl, dwState, dwWin32Err,    \
                                         dwSpecificErr, bHandled,           \
                                         dwEventType, lpEventData,          \
                                         lpContext ); break; }

//////////// SFL CLASS DECLARATION //////////////////////////////////////////
//
#define SFL_DECLARE_SERVICECLASS_FRIENDS(TService)                          \
    friend class CServiceBaseClass;                                         \

#define SFL_DECLARE_SIMPLESERVICEAPP_CLASS(TAppClass) class TAppClass: public CServiceAppT<TAppClass> {};

namespace SFL {
	typedef class _TSimpleApp: public CServiceAppT<_TSimpleApp> 
	{
	} CSimpleServiceApp;
}

#endif // IV_SFLBASE_H
