/*********************************************************************************/
/*                                                                               */
/*                                  DexTracker.h                                 */
/*                                                                               */
/*********************************************************************************/

/*
 * Interface to the DEX hardware.
 */

#pragma once

#include "../Useful/Useful.h"
#include "../Useful/Timers.h"
#include "../VectorsMixin/VectorsMixin.h"


/********************************************************************************/

#define N_MARKERS	24
#define N_CODAS		2

#define DEX_MAX_CODAS			8
#define DEX_MAX_MARKER_FRAMES	20000

// Structures that hold the collected data.


typedef struct {

	Vector3	position;
	bool	visibility;

} CodaMarker;

typedef struct {

	CodaMarker	marker[N_MARKERS];
	double		time;

} CodaFrame;

typedef struct {

	Vector3		position;
	Quaternion	orientation;
	bool		visibility;
	double		time;

} ManipulandumState;
class DexTracker : public VectorsMixin {

	private:

	protected:

	public:

		int nCodas;
		// Number of markers to be acquired.
		int nMarkers;
		int nAcqFrames;

		double samplePeriod;

		DexTracker() : nCodas( N_CODAS ), nMarkers( N_MARKERS ), samplePeriod( 0.005 ) {} ;

		virtual void Initialize( void );
		virtual int  Update( void );
		virtual void Quit( void );

		virtual void	StartAcquisition( float max_duration );
		virtual void	StopAcquisition( void );
		virtual bool	CheckAcquisitionOverrun( void );

		virtual int		RetrieveMarkerFrames( CodaFrame frames[], int max_frames, int unit = 0 );
		virtual bool	GetCurrentMarkerFrame( CodaFrame &frame );
		virtual bool	GetCurrentMarkerFrameUnit( CodaFrame &frame, int unit );
		virtual bool	GetCurrentMarkerFrameIntrinsic( CodaFrame &frame, int unit );

		virtual double	GetSamplePeriod( void );
		virtual int		GetNumberOfCodas( void );
		virtual bool	GetAcquisitionState( void );
		virtual void	GetUnitPlacement( int unit, Vector3 &pos, Quaternion &ori ) ;
		virtual void	GetUnitTransform( int unit, Vector3 &offset, Matrix3x3 &rotation ) ;
		virtual int		PerformAlignment(  int origin, int x_negative, int x_positive, int xy_negative, int xy_positive ) ;

		void			CopyMarkerFrame( CodaFrame &destination, CodaFrame &source );

};

/********************************************************************************/

class DexMouseTracker : public DexTracker {

private:

	bool		acquisitionOn;
	bool		overrun;
	Timer		acquisitionTimer;
	double		duration;
	int			nPolled;

	HWND		dlg;

	FILE		*fp;

	CodaFrame	polledMarkerFrames[DEX_MAX_MARKER_FRAMES];

	
protected:

public:

	DexMouseTracker( HWND dlg = NULL ) : acquisitionOn(false), overrun(false) {
		this->dlg = dlg;
	}

	void Initialize( void );
	void Quit( void );

	int  Update( void );
	void StartAcquisition( float max_duration );
	void StopAcquisition( void );
	bool GetAcquisitionState( void );
	bool CheckOverrun( void );

	int	 RetrieveMarkerFrames( CodaFrame frames[], int max_frames, int unit );
	bool GetCurrentMarkerFrame( CodaFrame &frame );
	bool GetCurrentMarkerFrameUnit( CodaFrame &frame, int unit );

	void GetUnitTransform( int unit, Vector3 &offset, Matrix3x3 &rotation );
	int  PerformAlignment( int origin, int x_negative, int x_positive, int xy_negative, int xy_positive );

};


// Use main Codamotion RTNet namespace
using namespace codaRTNet;

class DexRTnetTracker : public DexTracker {

private:

	// Hardwire the server IP address and port.
	char *serverAddress;
	unsigned int serverPort;

	// Marker tracker device.
	const int cx1Device;	// Should be the CX1

	// How many tries to get a data packet before giving up.
	int maxRetries;

	// Flag to keep track of overrunning the acquisition time.
	bool overrun;

	// Helper function to print network connection error codes (client-side errors)
	void print_network_error(const NetworkException& exNet);

	// Helper function to print device error codes (server-side errors)
	void print_devicestatusarray_errors(const DeviceStatusArray& status);

	// Helper function to print system alignment status
	int print_alignment_status(const DWORD* marker_id_array, const DeviceInfoAlignment& info);

	//* Generic data packet
	RTNetworkPacket packet;

	// client connection object
	RTNetClient cl;

	int codaConfig;
	DeviceOptionsCodaMode mode;
	DeviceOptionsCodaPacketMode packet_mode;

	// decoder objects
	PacketDecode3DResultExt decode3D;	// 3D measurements (CX1)
	PacketDecodeADC16		decodeADC;		// 16-bit ADC measurements (GS16AIO)

	// Various objects
	AutoDiscover	discover;


  // Holds information about the different configurations defined on the CODA system.
  // I am guessing that there will only be one. Ideally, though, one might define
  // three different configurations, one with both Coda units active, the other 
  // two with each of the Codas working in isolation.
	HWConfigEnum	configs;
	DataStream		stream;
	CODANET_HWCONFIG_DEVICEENABLE devices;

	CodaFrame		recordedMarkerFrames[DEX_MAX_CODAS][DEX_MAX_MARKER_FRAMES];

protected:

public:

	DexRTnetTracker( void ) : 
		// Host address and UDP port for the Coda RTnet server.
		serverAddress("192.168.1.1"), 
		serverPort(10111), 
		// Marker acquistion rate (200Hz), down sampling (none) and external sync (no).
		mode( CODANET_CODA_MODE_200, 1, false ), 
		// Request marker data from each Coda unit separately, and the combined data.
		packet_mode( CODANET_CODAPACKETMODE_SEPARATE_AND_COMBINED_COORD ),	
		// Use the first Coda configuration in the list.
		// This has to be set up as a cx1 only configuration on the server.
		codaConfig(1), 
		// A Coda RTnet configuration can include cx1 devices, ADC, force platforms, etc.
		// This is just a constant specifying the cx1 device.
		cx1Device(1),
		// This determines how many times we try to get a failed packet before giving up.
		maxRetries(5)
	{}

	void Initialize( void );
	int  Update( void );
	void StartAcquisition( float max_duration );
	void StopAcquisition( void );
	bool CheckAcquisitionOverrun( void );

	bool GetAcquisitionState( void );
	int  GetNumberOfCodas( void );

	int		RetrieveMarkerFrames( CodaFrame frames[], int max_frames, int unit );
	bool	GetCurrentMarkerFrame( CodaFrame &frame );

	// Need to add the following.
	void GetUnitTransform( int unit, Vector3 &offset, Matrix3x3 &rotation );
	int  PerformAlignment( int origin, int x_negative, int x_positive, int xy_negative, int xy_positive );

};


#if 0

// A version of a CODA tracker that uses the old SDK.

/********************************************************************************/

class DexCodaTracker : public DexTracker {

private:

	/* 
	 * Real-time marker data
	 * Defines storage for one Coda frame.
	 */
	CODA_FRAME_DATA_STRUCT coda_data_frame;
	codaBYTE bInView[ CODA_MAX_MARKERS ];
	codaFLOAT fPosition[ CODA_MAX_MARKERS * 3 ];

	/* 
	 * Recorded marker data
	 * Defines storage for one trial's worth of data.
	 */
	CODA_ACQ_DATA_MULTI_STRUCT coda_multi_acq_frame;
	codaBYTE bInViewMulti[ CODA_MAX_MARKERS * DEX_MAX_MARKER_FRAMES];
	codaFLOAT fPositionMulti[ CODA_MAX_MARKERS * DEX_MAX_MARKER_FRAMES * 3 ];

	bool overrun;


protected:

public:

	DexCodaTracker( void ){}
	
	void Initialize( void );
	int  Update( void );
	void StartAcquisition( float max_duration );
	void StopAcquisition( void );
	bool CheckAcquisitionOverrun( void );

	bool GetAcquisitionState( void );
	int  GetNumberOfCodas( void );

	int		RetrieveMarkerFrames( CodaFrame frames[], int max_frames );
	bool	GetCurrentMarkerFrame( CodaFrame &frame );

};

/********************************************************************************/
#endif

