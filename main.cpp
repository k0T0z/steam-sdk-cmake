#include <public\steam\steam_api.h>
#include <iostream>
#include "main.h"
#include <windows.h>

using namespace std;

int sum(int a, int b) {
	return a+b;
}

// bool steam_api;
// int app_id;
// int user_id;
// int64 u_id;

/*
When using the Steam API, this function can be called to check that the Steam client API has been 
initialised correctly before any doing any further calls to Steam specific functions in your game.
*/
// bool steam_initialised();
// bool steam_stats_ready();
// bool steam_is_overlay_enabled();

/*
This function is used retrieve the unique app ID that Steam assigns for your game, 
which is required for using some of the User Generated Content functions.
*/
// int steam_get_app_id();

/*
This function is used retrieve the unique User ID that Steam assigns to each user, 
which is required for using some of the User Generated Content functions.
*/
// int steam_get_user_account_id();

/*
You can use this function to return the unique Steam user id of the user currently logged into the Steam client. 
If you need to get the user's on screen user name you should refer to the function steam_get_persona_name.
*/
// int64 steam_get_user_steam_id();


extern "C" void __cdecl SteamAPIDebugTextHook( int nSeverity, const char *pchDebugText )
{
	// if you're running in the debugger, only warnings (nSeverity >= 1) will be sent
	// if you add -debug_steamapi to the command-line, a lot of extra informational messages will also be sent
	::OutputDebugString( pchDebugText );

	if ( nSeverity >= 1 )
	{
		// place to set a breakpoint for catching API errors
		int x = 3;
		(void)x;
	}
}

#define LittleInt16( val )	( val )
#define LittleWord( val )	( val )
#define LittleInt32( val )	( val )
#define LittleDWord( val )	( val )
#define LittleQWord( val )	( val )
#define LittleFloat( val )	( val )

enum EMessage
{
	// Server messages
	k_EMsgServerBegin = 0,
	k_EMsgServerSendInfo = k_EMsgServerBegin+1,
	k_EMsgServerFailAuthentication = k_EMsgServerBegin+2,
	k_EMsgServerPassAuthentication = k_EMsgServerBegin+3,
	k_EMsgServerUpdateWorld = k_EMsgServerBegin+4,
	k_EMsgServerExiting = k_EMsgServerBegin+5,
	k_EMsgServerPingResponse = k_EMsgServerBegin+6,

	// Client messages
	k_EMsgClientBegin = 500,
	k_EMsgClientBeginAuthentication = k_EMsgClientBegin+2,
	k_EMsgClientSendLocalUpdate = k_EMsgClientBegin+3,

	// P2P authentication messages
	k_EMsgP2PBegin = 600, 
	k_EMsgP2PSendingTicket = k_EMsgP2PBegin+1,

	// voice chat messages
	k_EMsgVoiceChatBegin = 700, 
	//k_EMsgVoiceChatPing = k_EMsgVoiceChatBegin+1,	// deprecated keep alive message
	k_EMsgVoiceChatData = k_EMsgVoiceChatBegin+2,	// voice data from another player



	// force 32-bit size enum so the wire protocol doesn't get outgrown later
	k_EForceDWORD  = 0x7fffffff, 
};

struct MsgP2PSendingTicket_t
{
	MsgP2PSendingTicket_t() : m_dwMessageType( LittleDWord( k_EMsgP2PSendingTicket ) ) {}
	DWORD GetMessageType() { return LittleDWord( m_dwMessageType ); }

	void SetToken( const void *pToken, uint32 unLen ) { m_uTokenLen = LittleDWord( unLen ); memcpy( m_rgchToken, pToken, min( unLen, sizeof( m_rgchToken ) ) ); }
	uint32 GetTokenLen() const { return LittleDWord( m_uTokenLen ); }
	const char *GetTokenPtr() const { return m_rgchToken; }

	// Sender or receiver (depending on context)
	void SetSteamID( uint64 ulSteamID ) { m_ulSteamID = LittleQWord( ulSteamID ); }
	uint64 GetSteamID() const { return LittleQWord( m_ulSteamID ); }

private:
	DWORD m_dwMessageType;
	uint32 m_uTokenLen;
	char m_rgchToken[1024];
	uint64 m_ulSteamID;
};

class CGameManager
{
public:
	STEAM_CALLBACK( CGameManager, OnGameOverlayActivated, GameOverlayActivated_t, m_CallbackOverlayReceived );

	CGameManager();

};

CGameManager::CGameManager() : m_CallbackOverlayReceived( this, &CGameManager::OnGameOverlayActivated ) {

}

void CGameManager::OnGameOverlayActivated( GameOverlayActivated_t* pCallback )
{
	if ( pCallback->m_bActive )
		cout <<  "Steam overlay now active" << endl;
	else
		cout << "Steam overlay now inactive" << endl;
}

int main() {
    if ( SteamAPI_RestartAppIfNecessary( k_uAppIdInvalid ) ) // Replace with your app id
	{
		return EXIT_FAILURE;
	}

	if ( !SteamAPI_Init() )
	{
		printf( "Fatal Error - Steam must be running to play this game (SteamAPI_Init() failed).\n" );
		return EXIT_FAILURE;
	}

	SteamClient()->SetWarningMessageHook( &SteamAPIDebugTextHook );

	return 0;

	// SteamFriends()->ActivateGameOverlay( "Settings" );

	bool isOverlayEnabled = SteamUtils()->IsOverlayEnabled();
	cout << "isOverlayEnabled: " << boolalpha << isOverlayEnabled << endl;

	// Get the current users Steam name.
	const char *name = SteamFriends()->GetPersonaName();
	cout << "username: " << name << endl;

	if ( !SteamUser()->BLoggedOn() )
	{
		return EXIT_FAILURE;
		
	}

	CSteamID cSteamID = SteamUser()->GetSteamID();

	uint32 m_cubTicketIGaveThisUser;
	uint8 m_rgubTicketIGaveThisUser[1024];
	uint32 m_cubTicketHeGaveMe;
	uint8 m_rgubTicketHeGaveMe[1024];
	HAuthTicket m_hAuthTicketIGaveThisUser;
	EBeginAuthSessionResult m_eBeginAuthSessionResult;
	EAuthSessionResponse m_eAuthSessionResponse;

	if ( m_cubTicketIGaveThisUser == 0 )
	{
		m_hAuthTicketIGaveThisUser = SteamUser()->GetAuthSessionTicket( m_rgubTicketIGaveThisUser, sizeof( m_rgubTicketIGaveThisUser ), &m_cubTicketIGaveThisUser );
	}

	EBeginAuthSessionResult response = SteamUser()->BeginAuthSession(m_rgubTicketHeGaveMe, m_cubTicketHeGaveMe, cSteamID);	
	char rgch[128];
	sprintf( rgch, "P2P:: ReceivedTicket from account=%d \n", cSteamID.GetAccountID() );
	OutputDebugString( rgch );

	SteamFriends()->ActivateGameOverlay("friends");

	/* set a global variable to true if the Steam client API is correctly initialised, 
	   along with the Steam statistics and overlay functionality, or it will set the variable to false otherwise. */
	// steam_api = false;
	// if (steam_initialised())
	// {
    // 	if (steam_stats_ready() && steam_is_overlay_enabled())
    // 	{
    //     	steam_api = true;
    // 	}
	// }

	// gets the unique app ID for your game on Steam and stores it in a global variable.
	// app_id = steam_get_app_id();

	// gets the unique user ID for the person who owns the game and stores it in a global variable.
	// user_id = steam_get_user_account_id();

	// set a global variable to the current users unique Steam ID if the Steam client API is correctly initialised.
	// if (steam_initialised())
	// {
    // 	u_id = steam_get_user_steam_id();
	// }

	SteamAPI_Shutdown();

	return 0;
}

// bool steam_initialised() {
	

// }

// bool steam_stats_ready() {
	

// }

// bool steam_is_overlay_enabled() {
	

// }

// int steam_get_app_id() {

// }

// int steam_get_user_account_id() {

// }

// int64 steam_get_user_steam_id() {

// }

