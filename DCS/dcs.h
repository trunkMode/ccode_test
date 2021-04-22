#ifndef SONICPOINT_AUTO_CHANNEL_POWER_H__

#define SONICPOINT_AUTO_CHANNEL_POWER_H__

 

//#include <listLib.h>  //OS

#include "singlyLinkedList.h"

#include <stdio.h>

#include "sp_api.h"

/*use

SonicPointNBand

SonicPointNMode

*/

#include "sonicPointObject.h"              

 

/**adjacent threshold*/

#define RSSI_THRESHOLD_MINI                 15

/*we want to get a scheme since we don't want to adjust the power at first*/

#define RSSI_THRESHOLD_MAX                  100

#define RSSI_THRESHOLD_DELTA                20

#define RSSI_STOP_INTERVAL                  5

#define RSSI_ACCEPTABLE_INTERVAL            6

/** tx power down or up*/

#define TX_POWER_UP     1

#define TX_POWER_DOWN   0

/*for setting channel or txpower*/

#define SET_POWER       1

#define SET_CHANNEL     2

 

/**pipe*/

#define ACP_DYNAMIC_CHANNEL_PIPE            "/pipe/dynamicChannel"

#define ACPTASK_PIPE_MAX_MSGS               128

#define ACPTASK_PIPE_UNIT                   sizeof( SpPtGuiMsg )

#define CALCTASK_PIPE                       "/pipe/calcPipe"

#define CALCTASK_PIPE_MAX_MSGS              16

#define CALCTASK_PIPE_UNIT                  sizeof(int)

 

/** pipe command:ACP_DYNAMIC_CHANNEL_PIPE*/

/* command:defined in sp_api.h*/

// #define SP_ADD_NEW_ONE              0x01

// #define SP_DEL_OLD_ONE              0x02

// #define SP_START_AUTO_CHANNEL       0x03

 

#define SPCS_DEVICE_NONE        0xff

 

#define NUM_USING_CHANNEL                   11

/*we only select 1,6 and 11 currently.To be extended.*/

#define OPTIONAL_CHANNEL_NUM                3  

#define NUM_KIND_COLOR_SCHEME               20

/*dca schedule param*/

typedef struct{

    uint32  dcaSchedHandle;     /*flash*/

    uint32  dcaSchedOldHandle;

    uint32  dcaScanAgingTime;   /*flash*/

    uint32  dcaScanMode;        /*flash*//*DCA head scan mode*/

    /*latency time for waiting new sp to join*/

    unit32  dcaLatencyTimeforNewSP;

    /*to be continued*/

}sDCA_Sched_Param;

/*dca param*/

typedef enum{

    SCANMOED_NONE_INITIAL                = 0x00,

    /*DCA head*/

    SCANMOED_FORCE_ALL_SP_SCAN           = 0x01,    /*vaule should be changed according to the value of option on web 'wlanSonicPointDCA.html'.*/

    SCANMOED_ALL_SP_SCAN_BASED_ON_TIME   = 0x02,

    /*DCA node*/

    SCANMODE_FORCE_THIS_SP_SCAN          = 0x03,

    SCANMODE_THIS_SP_SCAN_BASED_ON_TIME  = 0x04,

    SCANMOED_INVALID                     = 0xff

}SCAN_MODE_ENUM;

/*Moved to sp_api.h so that we can define some member in PeerList.*/

#if 1

typedef enum{

    CH_24G_20M_AUTO             = 0x00,

    CH_24G_20M_FIXED            = 0x01,

    CH_24G_40M_AUTO             = 0x02,

    CH_24G_40M_FIXED            = 0x03,

 

    CH_5G_20M_AUTO              = 0x04,

    CH_5G_20M_FIXED             = 0x05,

    CH_5G_40M_AUTO              = 0x06,

    CH_5G_40M_FIXED             = 0x07,

} CHANNEL_STATE_ENUM;

#endif

 

typedef enum{

    SSPP_RETURN_INVALID         = 0x00,

    SSPP_RETURN_OK              = 0x01,

    SSPP_RETURN_TIMEOUNT        = 0x02,

}SSPP_RETURN_STATE_ENUM;

/*rfScore of channel */

typedef struct{

    int rfScore;

    unsigned char chIndex;

}sRfNode;

 

typedef struct{

    int                 rfScore[NUM_USING_CHANNEL+1];

    unsigned char       rfaCh[NUM_USING_CHANNEL+1];               /*which channel the rfa based on*/

}sACPRfa;

 

typedef struct{

    CHANNEL_STATE_ENUM  enum_modeAndBand;

    SonicPointNMode     enum_radioMode;

    SonicPointNBand     enum_radioBand;

    unsigned char       ch[SONICPOINT_NUM_RADIOS];  /*primary and secondary channel*/

}sRadioInfo;

 

/*ACP node*/

typedef struct sAutoChannelAndPower{

    struct sAutoChannelAndPower     *next;

 

    sRadioInfo              s_radioInfo[SONICPOINT_NUM_RADIOS]; /*radio info*/

    sACPRfa                 s_rfa;                              /*do rfa when needed*/

    SonicPointWorkState     spsWorkState;

    SSPP_RETURN_STATE_ENUM  m_scanReturn;                       /* the scan status returned*/

    SCAN_MODE_ENUM          enum_spScanMode;/* take effection only when headDCA.enum_spScanMode=SCANMOED_ALL_SP_SCAN_BASED_ON_TIME*/

 

    unsigned char           BSSID[SONICPOINT_BSSID_LENGTH];     /* idMAC+1 ==>BSSID*/

    SonicPointVapBssid      vapBssidList;

//     char                    vapState;                           /*to reduce the count of searching in the scanning result*/

 

    unsigned char           optionalCh[OPTIONAL_CHANNEL_NUM];   /*optional channel*/

    unsigned short          txPowerLvlCurr; /* current tx power in terms of TP_SCALE_XXX */

    unsigned short          txPowerLvlPrev; /* previous tx power in terms of TP_SCALE_XXX */

    unsigned char           dynamicChAssign[SONICPOINT_NUM_RADIOS][SONICPOINT_NUM_RADIOS];

    unsigned char           nEnableDCA;     /* get EnableDCA form peerList.*/

   

    long int                scanTime;

}AutoChannelAndPower, *PAutoChannelAndPower;

 

/*txpower classification struct*/

typedef struct{

    SlList                  txPowerList[5]; /*0-50mw, 1-31mw, 2-15mw, 3-7mw, 4-1mw*/

}sTxPowerClassification;

/*txpower Node */

typedef struct TxPowerNode{

    struct TxPowerNode      *next;

    unsigned char           BSSID[SONICPOINT_BSSID_LENGTH];

    unsigned short          txPowerLvlCurr;

}sTxPowerNode;

/*color scheme*/

typedef struct{

    unsigned char   *pVertexColor;  /*points to memory which records colors of every vertexs.(note:dynamic allocate)*/

//     int             kindOfScheme;   /*the number of probable scheme--row*/

    int             vertexIndex;    /*used as the index for sending every channel to AP using sspp.*/

    int             schemeCnt;      /*to count the number of the probable assignment*/

    int             numOfVertex; /*pAssignment[kindOfScheme][col]*/

}sColorAssignment;

/*rfa for rf channel used by unmanaged aps and designed by user.*/

typedef struct{

    unsigned long       m_scanResultRSSI[SONICPOINT_SCANRESULT_MAX][SONICPOINT_NUM_RADIOS];/*spx--scan result*/

    unsigned short      chanFreq[SONICPOINT_SCANRESULT_MAX][SONICPOINT_NUM_RADIOS];        /*scan result only return one channel frequency*/

}sRFEvaluate;

 

typedef struct{

    int             delIndexArray[256];

    int             delCount;

    int             addCount;

    unsigned char   validIndex;

}sInfoUpdateMatrix;

/*ACP head*/

typedef struct sAutoChannelAndPowerHeader{

    SlList              slList;

    unsigned long       lastChAssignTime;

    SwSemId             semMutex;

    unsigned char       **pAdjacentMatrix;  /*adjust the dimension of matrix according to number of node */

    unsigned long       **pRSSIMatrix;      /*buffered the rssi maxtrix*/

    int                 m_matrixLen;        /*length of matrix*/

    int                 m_oldListCount;     /*updated during DCA calc and creation.*/

    int                 m_rssiCurThreshold; /*threshold for decision of adjacency or not*/

 

    sColorAssignment    s_colorScheme;      /*color assignmnet*/

    unsigned char       **pAPFixedCh;       /*sonicPoint & sonicPointNi/Ne & sonicPointNDR secondary NIC.*/

#if INCLUDE_SONICPOINT_NDR

    unsigned char       **pAPFixedChNDR;

#endif

 

    sRFEvaluate         s_rfEvaluate;       /*used as buffer to do rfa for every ACP node*/

    unsigned char       findFlag[SONICPOINT_SCANRESULT_MAX];    /*flag to mark if there are unmanaged APS in the scan result list.*/

 

    unsigned char       isNeedDoRfa;        /*marks if doing Rfa 's needed or not.*/

    unsigned char       roundOfCalc;        /*keep record how many times we have tried so that we can decide the delta of the rssi threshold*/

    SCAN_MODE_ENUM      enum_spScanMode;

 

    sInfoUpdateMatrix   s_InfoUpdateMatrix;

}AutoChannelAndPowerHeader;

 

/*ACP flag*/

typedef struct sACPFlag{

    SwSemId             semMutex;

    int                 m_calcTaskRunning;  /*1--running, 0--not started*/

    int                 m_acpTaskRunning;

    int                 m_addSPFromFlashOk; /*set it when initialize peerList sucessfully*/

}sACPGlobalData;

 

/*pipe:command*/

typedef enum

{

    CALCTASK_NO_CMD                     = 0x00,

    ACP2CALC_STOP_CALC_CMD              = 0x01,

    ACP2CALC_START_CALC_CMD             = 0x02,

    SSPPSCAN_RETURN                     = 0x03,

    ADJUST_TXPOWER_CMD_FINISH           = 0x04,

    SET_CHANNEL_CMD_FINISH              = 0x05,

    ADJUST_TXPOWER_CHANNEL_CMD_FINISH   = 0x06,

} CALCTASK_PIPE_CMD_ENUM;

 

typedef struct

{

  int m_startCalcCmdCnt;      /*count of starting calculating command*/

  int m_stopCalcCmdCnt;       /*count of stop calculating command*/

  int m_ssppScanReturn;

}CALCTASK_PIPE_CMD_CNT;

 

typedef enum

{

  STOP_CALC                   = 1,

  AP_IS_STABLE                = 2,

  NO_NEED_TO_WAIT             = 3,

  WAIT_AP_TO_STABLE_TIMEOUNT  = 4,

  NO_AP_CONNECTED             = 5,

} AP_STATES;

/*latency time of waitint for new sp to join*/

#define DYNAMIC_CHANNEL_TIMEOUT             90

#define DYNAMIC_CHANNEL_TIMEOUT_MIN         30

#define DYNMAIC_CHANNEL_TIMEOUT_MAX         600

 

#define CLACTASK_PIPE_TIMEOUT               6

/*interval of scanning(max) = 24 hours*/

#define DCA_SCAN_MAX_INTERVAL               24*60

#define DCA_SCAN_MIN_INTERVAL               1

#define DCA_SCAN_DEFAULT_INTERVAL           30

 

#define AUTO_CHANNEL_SP                     0

#define SSPP_TIMEOUT_CNT                    15

/*managed APS with fixed channels and unmanaged APS*/

#define TAKEN_INTO_RFA                      1

#define NOT_TAKEN_INTO_RFA                  0

 

/*radio unit*/

#define RADIO_UNIT0                         0

#define RADIO_UNIT1                         1

/*the max num of vap*/

#define MAX_NUM_VAP                         8

 

/*DCA node status*/

#define DCA_NO_NODE                         1

#define DCA_NODE_UPDATED_SUCESS             2

#define DCA_NODE_UPDATED_FAIL               3

 

/*allocate matrix type*/

#define TYPE_UNSIGNED_CHAR                  1

#define TYPE_SIGNED_CHAR                    2

#define TYPE_UNSIGNED_INT                   3

#define TYPE_SIGNED_INT                     4

#define TYPE_UNSIGNED_LONG                  5

 

/*start, stop or timeount for sspp*/

#define LAST_SSPP_TIMEOUNT                  0

#define LAST_SSPP_COMPLETED                 1

#define DCA_STOP_CALC                       -1

/* semaphore*/

#define DCA_DATA_PROTECT( semMutx )     SW_SEM_TAKE(semMutx);

 

#define DCA_DATA_RELEASE( semMutx )  {          \

    if(semMutx == NULL)                         \

    {                                           \

        printf(("semMutx is NULL"));            \

    }                                           \

    else                                        \

    {                                           \

        SW_SEM_GIVE(semMutx);                   \

    }                                           \

}

#define ID_TO_BSSID( mac )      mac[5]+=1;

#define BSSID_TO_ID( mac )      mac[5]-=1;

 

#ifndef MAC_STR_FORMAT

#define MAC_STR_FORMAT  "%2.2x:%2.2x:%2.2x:%2.2x:%2.2x:%2.2x"

#define MAC_ARRARY(x)    (unsigned char)(x)[0],(unsigned char)(x)[1],(unsigned char)(x)[2],(unsigned char)(x)[3],(unsigned char)(x)[4],(unsigned char)(x)[5]

#endif

 

/* external data*/

extern SlList       gSonicPointSlList;

extern int          gPtToGuiMsgFd;

extern SwSemId      semSdpMutx;         /* SDP_PROTECT */

extern SpRfaTree    g_spRfaTree;     /*rfa*/

 

extern unsigned long raw_truerand();

extern int rfaBuildSpList(BList *inBList);

extern int rfaFreeList();

extern DBSTATIC SwErrCode rfaInitIndexList( BList **inBList );

#if INCLUDE_SONICPOINT_11N

extern PSdpPeer sdpPeerSearch(UCHAR *macAddr, int addNew, BlNetId intf, char *buffer, UINT8 deviceType);

#else

extern PSdpPeer sdpPeerSearch(UCHAR *macAddr, int addNew, BlNetId intf, char *buffer);

#endif /* INCLUDE_SONICPOINT_11N */

 

#endif

 
