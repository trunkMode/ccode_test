/**

* @Feature:  auto channel and auto power

* @date:    2012/7/15

* @record:

    1) no sp limitation.

    2) old calculation: give all probable result and select one result based on constraints.(eg:fixed channels or channels used by uncontrollable APS)

    3) 2012/8/21

        $Change Calculation

            => start from the beginning of the node to calculate ,taking all constraits into consideration.

            => there is an optional channel in each AP node based on rfa and random mechanism.

        $Semaphore

            => take swSemCreate() instead of semCreate().(init:fw/system/semaphore.c  semInit())

        $GUI

            => GUI start button send command to ACPTask to start calculation.

        $NOTE:starting sequence

            => peerList initialization --> ACPTask --> sdpMainTask . (important)

    4) 2012/8/27

        $Add set channel and tx power interface

            =>SSPP part adds cases and sonicPoint Part.

    5) 2012/9/4

        $[SonicPoint] save the dynamic channel into apcfg so that the AP can be restored to dynamic channel when it restarts.(apCfgTable.)

                      set txpower(sspp).(NOTE: txpower 0-50mw, 1-31mw, 2-15mw, 3-7mw, 4-1mw.)

                      Add case to solve the problem that the channel and power setting will be restored to configuration saved in flash when excute IDS.

                      => ssppProcessScan()  if(DCA) {set DCA channel} else { retore to configuration files};

        $[FW] reset txpower of all Aps before calculation.

              set txpower according to MAC address.

    6) 2012/9/5

        $[FW] check if last sspp connection has been already completed and start next connection, which is very important when it comes to send different commands to the same AP at the same time, or it'll lost the subsequent commands.

    7) 2012/9/11

        $[FW] revise code to find the ideal threshold using dichotomy so as to close in ideal result more quickly.

        $[FW] revise calcTask to scan 2 times to get more accurate RSSI information.

              => add pRSSIMatrix.

    8) 2012/9/14

        $[FW] take the effection of sonicPoints which have already assigned channels to into account when do RFA of sonicPoints whose channel haven't been assigned.

        $[FW] assign secondary channel based on rfaScore[ch-4] and rfaScore[ch+4] to support radioBand=40MHZ.

    9) 2012/9/19--2012/9/20

        $[FW] add schedule for DCA.(sonicPointN)????

        $[FW] fix bug: If it's timeout to wait the scanning result when SP is under operational state, the recursion function 'giveOneAssign' will get lost in dead loop.

    10) 2012/9/21

        $[FW] doRfa():  pACPNode->s_rfa is based on unmanged APs and managed APs with fixed channels.

                        Add s_rfaTemp to calculate new rfa to decide optinal channels during the process of calculating channels.

        $[FW] add aging time for RSSI info of every sp or force all sp to scan.

        $[FW] fix Bug: <1> doRfa() It'll fail to access the assigned channels as pACPNodeTemp->dynamicChAssign[1][0] is no valued set at that moment.

                      <2> we should substitute it with headDCA.s_colorScheme.pVertexColor[pos].

    11)2012/9/26

        $[FW] fix Bug: RFA: abs(freqA-freqB)<20(MHZ) or abs(chA-chB)<=4

                       doRfa: recalculate score of ch1 to ch11.(remove break;)

        $[FW] getChNFromPeerList... update all sp's channel info.

    12)2012/10/8

        $[FW] scan based on time--> variable dimension of adjacent matrix and rssi matrix.[keep memory when sp has been deleted.][move backward when new sp has been added.]

    13)2012/10/9

        $[FW] DCAList insert sp node from the head of list, so chanage moveForwardMatrix into moveBackForwardMatrix.

        $[FW] clear data cell after moving matrix data backward.(it's important as old value may result in bad channel assignment.)

        $[FW] save scanning time only when the scanning result has been returned sucessfully.

        $[FW] Force sp to scan when sp state changes into operation.

    14)2012/10/10

        $[FW] add lastSpWorkStates and lastLastSpWorkStates to avoid restarting to scan.

        $[FW] fix Bug[assign channel to sp]: move to next assigned channel.(it's important to move forward whether sp operational or not.)

    15)2012/10/11

        $[FW] add editBox for configuring aging time for scanning result.[html,paramTable]

    16)2012/10/15

        $[FW] add optionBox for configuring DCA scan mode.

    17)2012/10/16

        $[FW] eliminate vap from the scanning result so as to get more accurate RFA. < spListPeerMBssid() sonicPointProvisionUtil.c >

        $[FW} to support sonicPoint?

              ->addWlanSonicPoint.html

    NOTE:

        $[FW] how to deal with sp whose radio has been disabled?( just leave it alone as we can still start scan and set up adjacent matrix.)

        $[FW] check if the radio is enabled or not?(don't check as the scan operation can still be executed.)

        $[FW] the disabled radio will not shut down after scanning operation.

    ToDo list.

        $[FW] to start DCA when the states of some SP are changing into operational state.

        $[FW] the effection of secondary channel when radio band's under 40MHZ mode.

        $[FW] to support sonicPoint.

        $[FW]support vap when enable DCA feature.

        $[NDR]?

        $[sensitivity] {low | medium | high}

        $[FW] save into flash??

*/

#include "os_api.h"

#include "taskHookLib.h"

#include "sonicPointDCA.h"

#include "sonicPointSdpCfg.h"

#include "swMemory.h"

#include "selectLib.h"

#include <stdarg.h>

#include <time.h>

#include "gestalt.h"

#include "netObj_api.h"

#include "schedObjTimer.h"

#include "scheduleObject.h"

#include "gst.h"

 

#if DEBUGALL

int     dca_debug = 2;

#   define DCA_DEBUG(dca_level, a)             { if (dca_debug >= dca_level) printf a; }

#   define DSTATIC

#else

/*; is for 'for' loop*/

#   define DCA_DEBUG(dca_level,a)            ;

#   define DSTATIC                 static

#endif

/** local definition*/

DSTATIC int                     m_justBoot                                  = 1;    /*auto channel for the first time*/

DSTATIC int                     m_channelAssignFlag                         = 0;

static int                      msgLen                                      = 0;

static int                      m_calcCmdArray[ CALCTASK_PIPE_MAX_MSGS ]    = {0};  /*read all pipe msgs once*/

static CALCTASK_PIPE_CMD_CNT    s_calcTaskPipeCmdCnt                        = { 0, 0, 0 };

static const int                channel[3]                                  = { 1, 6, 11 };

static const short              frequency24G[16] = { 0, 2412, 2417, 2422, 2427, 2432, 2437, 2442, 2447, 2452, 2457, 2462, 2467, 2472, 2477, 2484 };/*1~~11 channel*/

static sACPRfa                  s_rfaTemp;

 

/** global definition*/

int                             scanCnt             = 1;

int                             gDynamicChannelFd   = 0;    /*pipe: sdp <--->ACP*/

int                             gCalcTaskPipeFd     = 0;    /*pipe: sspp<--->calcTask<--->ACP*/

AutoChannelAndPowerHeader       headDCA ;

sACPGlobalData                  sACPFlag;

/*gui-schedule-handle*/

sDCA_Sched_Param                gDCASchedParam = { NO_HANDLE, NO_HANDLE, 1 , SCANMOED_FORCE_ALL_SP_SCAN};//min=1min.

 

uint32 getDCAScanAgingTime()

{

    uint32 tmp = 0;

 

    PARAM_TABLE_PROTECT;

    tmp = gDCASchedParam.dcaScanAgingTime;

    PARAM_TABLE_RELEASE;

    return tmp;

}

 

uint32 getDCALatencyTimeForNewSP()

{

    uint32 tmp = 0;

 

    PARAM_TABLE_PROTECT;

    tmp = gDCASchedParam.dcaLatencyTimeforNewSP;

    PARAM_TABLE_RELEASE;

    return tmp;

}

 

SwErrCode getAutoPowerLevel( char *mac, unsigned char *lvlA, unsigned char *lvlG )

{

    PSpPeerList         spPeer;

 

    if( NULL == mac )

        return SW_ERR_FAIL;

 

    SDP_PROTECT;

 

    if (( spPeer = searchPeerFromMacSonicPoint(mac)) != (PSpPeerList)0 )

    {

        *lvlA = spPeer->autoPowerLevel[0];

        *lvlG = spPeer->autoPowerLevel[1];

        SDP_RELEASE;

        return SW_ERR_SUCCESS;

    }

 

    SDP_RELEASE;

    return SW_ERR_FAIL;

}

/**

* @NOTE: called by web.

*/

int sendStartCalcCmd()

{

    SpPtGuiMsg  msg;

    DCA_DEBUG( 1, ("SP: write to dynamic channel by GUI \n") )

    msg.msgCmd = SP_START_AUTO_CHANNEL;

    memset( msg.msgMac, 0x00, sizeof(msg.msgMac) );

    write( gDynamicChannelFd, (char *)&msg, sizeof(SpPtGuiMsg) );

    return 0;

}

/*-------------------------------DCA schedule (part)-----------------------------------------------------------*/

/*sonicPointN/sonicPointNDR DCA schedule*/

SchedObjTimerErr DCASchedTimer_EventCbStart( struct SchedObjTimer *soTimer, SchedEventCb *cb, void *data )

{

    DCA_DEBUG( 4, ("calling DCA schedule start ...\n") )

    sendStartCalcCmd();

 

    return SCHED_OBJ_TIMER_ERR_NONE;

}

 

SchedObjTimerErr DCASchedTimer_EventCbEnd( struct SchedObjTimer *soTimer, SchedEventCb *cb, void *data )

{

    DCA_DEBUG( 4, ("calling DCA schedule End ...\n") )

    return SCHED_OBJ_TIMER_ERR_NONE;

}

/*DCA schedule timer callbacks.*/

SchedEventCb DCASchedTimer_EventCb =

{

    "DCA SchedTimer event CB",

    DCASchedTimer_EventCbStart,

    DCASchedTimer_EventCbEnd

};

DBSTATIC bool DCASchedObj_DelPermit( uint32 handle, void * data, const char **msgId )

{

    const char *strInuse = GST("Schedule Object in use by DCA");

    if( msgId && *msgId )

        *msgId = strInuse;

 

    return FALSE;           /* don't permit, object in use */

}

/*DCA schedule obj callbacks.*/

NetObjCbTable DCASchedOBj_CbTable =

{

    "DCA ScheduleObj Callback",

    NULL,                   /* modifyPermit */

    NULL,                   /* modifyInform */

    NULL,                   /* widpSchedModDoneCb */

    DCASchedObj_DelPermit,

    NULL,                   /* deleteInform */

    NULL                    /* deleteDone */

};

/*unregister DCA schedule_objectTable_callback and schedule_timer_event_callback*/

void DCASchedUnregister( uint32 schedHandle )

{

    if( schedHandle != NO_HANDLE )

    {

        DCA_DEBUG( 4, ("<<< Unregistering DCA Schedule object %d ...\n", schedHandle) )

        schedObjTimer_unregisterCb( getSchedObjTimer(), schedHandle, &DCASchedTimer_EventCb, NULL );

        scheduleObjUnregisterCb( &scheduleObjectTable, schedHandle, &DCASchedOBj_CbTable, NULL );

    }

}

/*register DCA schedule_objectTable_callback and schedule_timer_event_callback*/

sw_bool DCASchedRegister( uint32 schedHandle )

{

    if( schedHandle != NO_HANDLE )

    {

        DCA_DEBUG( 4, (">>>> Registering DCA Schedule object %d ...\n", schedHandle) )

        scheduleObjRegisterCb( &scheduleObjectTable, schedHandle, &DCASchedOBj_CbTable, NULL );

        schedObjTimer_registerCb( getSchedObjTimer(), schedHandle, &DCASchedTimer_EventCb, NULL );

    }

    return sw_true;

}

void completeDCASchedule( uint32 oldSchedHandle, uint32 schedHandle )

{

    if( oldSchedHandle != schedHandle )

    {

        DCASchedUnregister( gDCASchedParam.dcaSchedOldHandle );

        DCASchedRegister( schedHandle );

        gDCASchedParam.dcaSchedOldHandle = schedHandle;

    }

    return;

}

/**

* @param:p to save the index of dcaNode in the linked-list.

*/

void *findSpByMAC( const unsigned char *pMAC , int *p)

{

    int index = 0;

    PAutoChannelAndPower pACPNode = ( PAutoChannelAndPower )( headDCA.slList.slHead.next );

    while(pACPNode)

    {

        if( 0 == memcmp( pMAC, pACPNode->BSSID, 6 ) )

        {

            if( p != NULL )

                *p = index;

            return (void*)pACPNode;

        }

        index++;

        pACPNode = pACPNode->next;

    }

    return NULL;

}

/**

* @func:get channel of sp or sp_NDR from PeerList.

* @param:radioUnit 0--NDR-second, 1--spN and NDR-first.

*/

DSTATIC void getChNFromPeerList( const SpPeerList *pPeerList, PAutoChannelAndPower pACPNode, int radioUnit )

{

    const SonicPointConfiguration *pCfg     = (SonicPointConfiguration *)&(pPeerList->spPrvCfg);

    int radioMainMode                       = pCfg->nRadio[radioUnit].mainRadioMode;

    int nRadioBand                          = pCfg->nRadio[radioUnit].nRadioBand;

    unsigned char *pChP                     = &pACPNode->s_radioInfo[radioUnit].ch[0];               //primary channel.

    unsigned char *pChS                     = &pACPNode->s_radioInfo[radioUnit].ch[1];               //secondary channel.

    CHANNEL_STATE_ENUM *enum_fixCh          = &pACPNode->s_radioInfo[radioUnit].enum_modeAndBand;

 

    pACPNode->s_radioInfo[radioUnit].enum_radioMode = radioMainMode;

    pACPNode->s_radioInfo[radioUnit].enum_radioBand = nRadioBand;

 

    if( RADIO_MODE_11G_ONLY_24 == radioMainMode )

    {

        /*gOnly--->gChannel*/

        *pChP = pCfg->nRadio[radioUnit].gChannel;

        *pChS = 0;

        if( *pChP > 0)

            *enum_fixCh = CH_24G_20M_FIXED;      /*fixed channel*/

        else

            *enum_fixCh = CH_24G_20M_AUTO;

    }

    else if( RADIO_MODE_11A_ONLY_50 == radioMainMode )

    {

        /*aOnly-->aChannel*/

        *pChP = pCfg->nRadio[radioUnit].aChannel;

        *pChS = 0;

        if( *pChP > 0)

            *enum_fixCh = CH_5G_20M_FIXED;       /*fixed channel*/

        else

            *enum_fixCh = CH_5G_20M_AUTO;

    }

    else if( RADIO_MODE_11N_ONLY_24 == radioMainMode || RADIO_MODE_11N_MIXED_24 == radioMainMode )

    {

        /*2.4G 11NOnly and 2.4G 11NMixed--->n24StandardChannel*/

        if( SPN_RADIO_BAND_AUTO == nRadioBand )

        {

            *pChP = 0;

            *pChS = 0;

            *enum_fixCh = CH_24G_40M_AUTO;     

        }

        else if( SPN_RADIO_BAND_STANDARD == nRadioBand )

        {

            /*2.4G band: 20Mhz standard*/

            *pChP = pCfg->nRadio[radioUnit].n24StandardChannel;

            *pChS = 0;

            if( *pChP > 0)

                *enum_fixCh = CH_24G_20M_FIXED;  /*fixed channel*/

            else

                *enum_fixCh = CH_24G_20M_AUTO;

        }

        else if (SPN_RADIO_BAND_WIDE == nRadioBand )

        {

            /*2.4G band:40Mhz wide*/

            *pChP = pCfg->nRadio[radioUnit].n24Primary;

            *pChS = pCfg->nRadio[radioUnit].n24Secondary;

            if( 0 == *pChP)

                *enum_fixCh = CH_24G_40M_AUTO;   /*40MHZ auto*/

            else

                *enum_fixCh = CH_24G_40M_FIXED;

        }

    }

    else if ( RADIO_MODE_11N_ONLY_50 == radioMainMode || RADIO_MODE_11N_MIXED_50 == radioMainMode )

    {

        /*5G 11NOnly and 5G 11NMixed--->n50StandardChannel*/

        if( SPN_RADIO_BAND_AUTO == nRadioBand)

        {

            *pChP = 0;

            *pChS = 0;

            *enum_fixCh = CH_5G_40M_AUTO;        /*default: 40MHZ auto*/

        }

        else if( SPN_RADIO_BAND_STANDARD == nRadioBand )

        {

             /* 5G band: 20Mhz standard*/

            *pChP = pCfg->nRadio[radioUnit].n50StandardChannel;

            *pChS = 0;

            if( *pChP > 0)

                *enum_fixCh = CH_5G_20M_FIXED;   /*fixed channel*/

            else

                *enum_fixCh = CH_5G_20M_AUTO;

        }

        else if(SPN_RADIO_BAND_WIDE == nRadioBand )

        {

            *pChP = pCfg->nRadio[radioUnit].n50Primary;

            *pChS = pCfg->nRadio[radioUnit].n50Secondary;

            if( 0 == *pChP)

                *enum_fixCh = CH_5G_40M_AUTO;    /*40MHZ auto*/

            else

                *enum_fixCh = CH_5G_40M_FIXED;

        }

    }

    return;

}

/**

* @func: update sp info.

* @return: 1 -- ok  0 -- failed

*/

DSTATIC int updateSPRelatedInfo()

{

    char mac[6] = { 0x00 };

    int i = 0;

    PAutoChannelAndPower    pACPNode = ( PAutoChannelAndPower ) headDCA.slList.slHead.next;

    PSpPeerList             pCurrentSpPeer = NULL;

 

    while( pACPNode )

    {

        memcpy( mac, pACPNode->BSSID, sizeof(pACPNode->BSSID) );

        BSSID_TO_ID( mac )                /*convert it into id*/

 

        SDP_PROTECT

        pCurrentSpPeer = ( PSpPeerList ) searchPeerFromMacSonicPoint( mac );

        if( NULL == pCurrentSpPeer )

            return 0;

        else

        {

            memcpy( &pACPNode->vapBssidList, &pCurrentSpPeer->vapBssidList, sizeof(SonicPointVapBssid) ); /*update vap list*/

//          pACPNode->vapState      = pCurrentSpPeer->spPrvCfg.vapState;

            pACPNode->nEnableDCA    = pCurrentSpPeer->spPrvCfg.nRadio[1].nEnableACP;    //NDR?

            pACPNode->spsWorkState  = pCurrentSpPeer->spStatus.spsWorkState;

            /*if ok, get info about RF at position 1.*/

            getChNFromPeerList( pCurrentSpPeer, pACPNode, RADIO_UNIT1 );  /*it needs to update all sp's channel info before calculating.*/

            headDCA.pAPFixedCh[i][0] = pACPNode->s_radioInfo[ RADIO_UNIT1 ].ch[0];

            headDCA.pAPFixedCh[i][1] = pACPNode->s_radioInfo[ RADIO_UNIT1 ].ch[1];

#if INCLUDE_SONICPOINT_NDR

            getChNFromPeerList( pCurrentSpPeer, pACPNode, RADIO_UNIT0 );

            headDCA.pAPFixedChNDR[i][0] = pACPNode->s_radioInfo[ RADIO_UNIT0 ].ch[0];

            headDCA.pAPFixedChNDR[i][1] = pACPNode->s_radioInfo[ RADIO_UNIT0 ].ch[1];

#endif  /*INCLUDE_SONICPOINT_NDR*/

            i++;

        }

        SDP_RELEASE

 

        pACPNode = pACPNode->next;

    }

    return 1;

}

/**

* @func: get random number ranging from randMin to randMax.

*/

DSTATIC int getRandomNumber( int randMin, int randMax )

{

    register int n = time(NULL) & 0xfff;

    for( ; n-- ; )

        rand();

    return rand() % ( abs(randMax - randMin) + 1 ) + ( randMin < randMax ? randMin : randMax );

}

#if 0

DSTATIC int getRandomNumber(int randMin, int randMax)

{

    return raw_truerand() * ( abs(randMax - randMin) + 1) + ( randMin < randMax? randMin : randMax );

}

#endif

/**

* @func: get random number with no reduplicate ranging form min to max.

* @param:[min,max] numOfGet--the count you want to get.

* @return:return number created.

*/

int getRandomWithoutReduplicate( int min, int max, int numOfGet, unsigned char *pOptionalCh, int opNum )

{

    int arraySize   = abs( max - min + 1 ); //the size of the array to put the cards.

    int *pArray     = (int*) SWMALLOC( arraySize * sizeof(int));//allocate the array for putting cards.

    int randomNum   = 0, i = 0, tmp = 0;

    if( NULL == pArray )

    {

        CRITICAL(("DCA: Failed to allaocate memory for array!"));

        return SW_ERR_FAIL;

    }

    /*initialize the card box, eg: if we want the number ranage from 3 to 10, the card in card box is as follows:3,4,5,6,7,8,9.*/

    for( i = 0 ; i < arraySize ; i++ )

            pArray[i] = i + min;

 

    if( numOfGet > arraySize )

        numOfGet = arraySize;

 

    for( i = 0; i < numOfGet; i++)

    {

        randomNum               = getRandomNumber( 0, arraySize-1-i );

        tmp                     = pArray[randomNum];

        pArray[randomNum]       = pArray[arraySize-1-i];

        pArray[arraySize-1-i]   = tmp;

    }

 

    for( i = 0; i < numOfGet; i++ )

    {

        if( 1 == pArray[arraySize-1-i] || 6 == pArray[arraySize-1-i] || 11 == pArray[arraySize-1-i] )

        {

            if( opNum >= 1 )  /*opNum=3, index=0,1,2*/

            {

                pOptionalCh[opNum-1] = pArray[arraySize-1-i];

                opNum--;

            }

        }

    }

    SWFREE(pArray);

    return numOfGet;

}

/**

* @ func: sort optional channel based on rfa result.

*/

DSTATIC void sort( sACPRfa *pACPRfa, unsigned char *pOpCh, int num )

{

    sRfNode rfNode;

    int i = 0, j = 0, k, count = 0;

 

    if( NULL == pACPRfa || NULL == pOpCh )

    {

        DCA_DEBUG(1,("ACP:[%s:%d]param is NULL!\n", __FUNCTION__, __LINE__))

        return;

    }

    for( i = 1; i <= NUM_USING_CHANNEL; i++ )

    {

        if( pACPRfa->rfScore[i] > 0 )   /*i--just treated as index here.*/

        {

            count++;

            break;

        }

    }

 

    if( count > 0 )

    {

        for( i = 1; i <= NUM_USING_CHANNEL; i++ )   /*i--just treated as index here.*/

        {

            k = i;

            for( j = i+1; j <= NUM_USING_CHANNEL; j++ )

            {

                if( pACPRfa->rfScore[k] > pACPRfa->rfScore[j] )

                    k = j;

            }

            if( k != i )

            {

                rfNode.rfScore      = pACPRfa->rfScore[k];

                rfNode.chIndex      = pACPRfa->rfaCh[k];

                pACPRfa->rfScore[k] = pACPRfa->rfScore[i];

                pACPRfa->rfaCh[k]   = pACPRfa->rfaCh[i];

                pACPRfa->rfScore[i] = rfNode.rfScore;

                pACPRfa->rfaCh[i]   = rfNode.chIndex;

            }

        }

        /*get out the pOpch(operational channel)*/

        k = 0;

        for( i = 1; i <= NUM_USING_CHANNEL; i++ ) //1--11 index for rfaCh[index]

        {

            if( 1 == pACPRfa->rfaCh[i] || 6 == pACPRfa->rfaCh[i] || 11 == pACPRfa->rfaCh[i] )

            {

                if( k < num ) /*num=3, k=0,1,2*/

                    pOpCh[k++] = pACPRfa->rfaCh[i];

            }

        }

    }

    else

    {

        getRandomWithoutReduplicate( 1, 11, 11, pOpCh, num );

    }

#if DEBUGALL

    /*print out the result*/

    DCA_DEBUG(1, ("After sort:\n"))

    for( i = 1; i <= NUM_USING_CHANNEL; i++ )

    {

        DCA_DEBUG( 1, ("pACPRfa->rfScore[%d] = %d\n", pACPRfa->rfaCh[i], pACPRfa->rfScore[i] ))

    }

    for( i = 0; i < 3 ; i++ )

    {

        DCA_DEBUG(1,("optionalCh[%d] = %d\n", i, pOpCh[i]))

    }

#endif

    return;

}

 

DSTATIC void decideOptionalCh( PAutoChannelAndPower pACPNode , sACPRfa *pACPRfa)

{

    if( NULL == pACPNode )

        return;

 

    if( NULL == pACPRfa )

        sort( &pACPNode->s_rfa, pACPNode->optionalCh, OPTIONAL_CHANNEL_NUM );

    else

        sort( pACPRfa, pACPNode->optionalCh, OPTIONAL_CHANNEL_NUM );

    return;

}

/**

* @func: set power or channel of all APs.

* @NOTE: 1) reset txpower of all aps to 'val'.          2) set dynamic channel assignment of all aps.

         3) set one designated ap's channel to 'val'    4) set one designated ap's txpower to 'val'.

         4)the function should be called outside DCA_DATA_PROTECT(headDCA.semMutex), or there will be a deadlock.(*)

*/

void setPowerOrCh( const char *mac, int setPwrCh, int val)

{

    SpPtGuiMsg           msg;

    PAutoChannelAndPower pACPNode       = ( PAutoChannelAndPower ) headDCA.slList.slHead.next;

    PAutoChannelAndPower pACPNTmp       = NULL;

    PSpPeerList          pCurrentSpPeer = NULL;

    int                  nEnableACP     = 0;

    int                  i              = 0;

 

    DCA_DATA_PROTECT( headDCA.semMutex )

 

    if( NULL == mac)    /*set all aps*/

    {

        if( SET_CHANNEL == setPwrCh )

        {

            DCA_DEBUG( 1, ("DCA[ %s : %d ] Set all ap's channel!\n", __FUNCTION__, __LINE__))

            /*check if the scheme has been calculated.*/

            if( headDCA.s_colorScheme.schemeCnt > 0 )

            {

                msg.msgCmd = SP_PROVISION_CHANNEL_SET;

 

                while( pACPNode )

                {

                    if( SP_STATE_OPERATIONAL == pACPNode->spsWorkState )

                    {

                        if( headDCA.s_colorScheme.pVertexColor[i] != 0xff ) /*ensure the channel is valid*/

                        {

                            memcpy( msg.msgMac, pACPNode->BSSID, sizeof( pACPNode->BSSID ) );

                            BSSID_TO_ID( msg.msgMac )

 

                            SDP_PROTECT

                            pCurrentSpPeer = searchPeerFromMacSonicPoint( msg.msgMac );

                            if( NULL == pCurrentSpPeer )

                            {

                                DCA_DEBUG(1, ("DCA[ %s:%d ] can't find sp["MAC_STR_FORMAT"] node!\n", __FUNCTION__, __LINE__, MAC_ARRARY(msg.msgMac)))

                                SDP_RELEASE

                                break;

                            }

 

                            if( CH_24G_20M_AUTO == pACPNode->s_radioInfo[1].enum_modeAndBand || CH_5G_20M_AUTO == pACPNode->s_radioInfo[1].enum_modeAndBand )

                            {

                                pCurrentSpPeer->dynamicChAssign[1][0] = headDCA.s_colorScheme.pVertexColor[i]; //primary channel.

                                pCurrentSpPeer->dynamicChAssign[1][1] = 0xff;                                  //secondary channel.

                            }

                            else if( CH_24G_40M_AUTO == pACPNode->s_radioInfo[1].enum_modeAndBand || CH_5G_40M_AUTO == pACPNode->s_radioInfo[1].enum_modeAndBand  )

                            {

                                pCurrentSpPeer->dynamicChAssign[1][0] = headDCA.s_colorScheme.pVertexColor[i]; //primary channel.

                                /*how to assign the secondary channel , how to take it into account when calculate channel.*/

                                /*now we just add 4 or subtract 4*/

                                if( pCurrentSpPeer->dynamicChAssign[1][0] <= 4 )

                                    pCurrentSpPeer->dynamicChAssign[1][1] = pCurrentSpPeer->dynamicChAssign[1][0] + 4;

                                else if (pCurrentSpPeer->dynamicChAssign[1][0] >= 8 )

                                    pCurrentSpPeer->dynamicChAssign[1][1] = pCurrentSpPeer->dynamicChAssign[1][0] - 4;

                                else

                                {

                                    if( pACPNode->s_rfa.rfScore[ pCurrentSpPeer->dynamicChAssign[1][0]+4 ] > pACPNode->s_rfa.rfScore[ pCurrentSpPeer->dynamicChAssign[1][0]-4 ] )

                                        pCurrentSpPeer->dynamicChAssign[1][1] = pCurrentSpPeer->dynamicChAssign[1][0] - 4;

                                    else

                                        pCurrentSpPeer->dynamicChAssign[1][1] = pCurrentSpPeer->dynamicChAssign[1][0] + 4;

                                }

                            }

                            else

                            {

                                /*set invalid flag*/

                                pCurrentSpPeer->dynamicChAssign[1][0] = 0xff;

                                pCurrentSpPeer->dynamicChAssign[1][1] = 0xff;

                            }

 

                            if( SP_STATE_OPERATIONAL == pCurrentSpPeer->spStatus.spsWorkState )

                            {

                                /*NOTE:Actually, we'd better test if peerList.spSession == -1 ? */

                                write( gPtToGuiMsgFd, (char *)&msg, sizeof(SpPtGuiMsg) );

                            }

                            SDP_RELEASE

#if INCLUDE_SONICPOINT_NDR

//...

#endif  /*INCLUDE_SONICPOINT_NDR*/

                        }

                    }//end_if_check_work_state.

                    i++;    /*move to next assigned channel.(it's important to move forward whether sp operational or not.*/

                    pACPNode = pACPNode->next;

                }//end while_channel.

            }//end_if_schemeCnt>0.

#if DEBUGALL

            else

            {

                DCA_DEBUG( 1, ("DCA[ %s : %d ] Please do dynamic channel assignment first!\n", __FUNCTION__, __LINE__) )

            }

#endif

        }//end_if_SET_CHANNEL

        else if( SET_POWER == setPwrCh )

        {

            msg.msgCmd = SP_PROVISION_POWER_ADJUST;

 

            while( pACPNode )

            {

                DCA_DEBUG( 1, ("DCA[%s:%d] reset all ap's power!\n", __FUNCTION__, __LINE__) )

 

                if( SP_STATE_OPERATIONAL == pACPNode->spsWorkState )

                {

                    /*buffer mac and cmd*/

                    memcpy( msg.msgMac, pACPNode->BSSID, sizeof( pACPNode->BSSID ) );

                    BSSID_TO_ID( msg.msgMac )

 

                    pACPNode->txPowerLvlCurr = 0;

                    pACPNode->txPowerLvlPrev = 0;

 

                    SDP_PROTECT

                    pCurrentSpPeer = searchPeerFromMacSonicPoint( msg.msgMac );

                    if( NULL == pCurrentSpPeer )

                    {

                        DCA_DEBUG(1,("DCA[ %s:%d ] can't find sp["MAC_STR_FORMAT"] node!\n", __FUNCTION__, __LINE__, MAC_ARRARY(msg.msgMac)))

                        SDP_RELEASE

                        break;

                    }

 

                    pCurrentSpPeer->txPowerLvlCurr = pACPNode->txPowerLvlCurr;

                    pCurrentSpPeer->txPowerLvlPrev = pACPNode->txPowerLvlPrev;

 

                    if( SP_STATE_OPERATIONAL == pCurrentSpPeer->spStatus.spsWorkState )

                    {

                        /*NOTE:Actually, we'd better test if peerList.spSession == -1 ? */

                        write( gPtToGuiMsgFd, (char *)&msg, sizeof(SpPtGuiMsg) );

                    }

                    SDP_RELEASE

                }

                pACPNode = pACPNode->next;

            }

        }

    }//end(NULL == mac)

    else    /* set one ap*/

    {

        pACPNTmp = (PAutoChannelAndPower) findSpByMAC(mac, NULL);

        memcpy( msg.msgMac, mac, 6 );

        BSSID_TO_ID( msg.msgMac );

 

        SDP_PROTECT

        pCurrentSpPeer = searchPeerFromMacSonicPoint( msg.msgMac );

        if( NULL == pCurrentSpPeer )

        {

            DCA_DEBUG(1,("DCA[ %s:%d ] can't find sp["MAC_STR_FORMAT"] node!\n", __FUNCTION__, __LINE__, MAC_ARRARY(msg.msgMac)))

            SDP_RELEASE

            goto RETURN_SET_CH_POWER;

        }

        nEnableACP = pCurrentSpPeer->spPrvCfg.nRadio[1].nEnableACP;

 

        if( SP_STATE_OPERATIONAL == pCurrentSpPeer->spStatus.spsWorkState )

        {

            if( SET_CHANNEL == setPwrCh )

            {

                msg.msgCmd = SP_PROVISION_CHANNEL_SET;

 

                if( val >= 0 && val <= 11 ) /*validate the ch*/

                {

                    if( CH_24G_20M_AUTO == pACPNTmp->s_radioInfo[1].enum_modeAndBand || CH_5G_20M_AUTO == pACPNTmp->s_radioInfo[1].enum_modeAndBand )

                    {

                        pCurrentSpPeer->dynamicChAssign[1][0] = val;  //primary channel.

                        pCurrentSpPeer->dynamicChAssign[1][1] = 0xff; //secondary channel.

                    }

                    else if( CH_24G_40M_AUTO == pACPNTmp->s_radioInfo[1].enum_modeAndBand || CH_5G_40M_AUTO == pACPNTmp->s_radioInfo[1].enum_modeAndBand  )

                    {

                        pCurrentSpPeer->dynamicChAssign[1][0] = headDCA.s_colorScheme.pVertexColor[i]; //primary channel.

                        /*how to assign the secondary channel , how to take it into account when calculate channel.*/

                        /*now we just add 4 or subtract 4*/

                        if( pCurrentSpPeer->dynamicChAssign[1][0] < 8 )

                            pCurrentSpPeer->dynamicChAssign[1][1] = pACPNTmp->dynamicChAssign[1][0] + 4;

                        else

                            pCurrentSpPeer->dynamicChAssign[1][1] = pACPNTmp->dynamicChAssign[1][0] - 4;

                    }

                    else

                    {

                        /*set invalid flag*/

                        pCurrentSpPeer->dynamicChAssign[1][0] = 0xff;

                        pCurrentSpPeer->dynamicChAssign[1][1] = 0xff;

                    }

 

                    if( 1 == nEnableACP )

                        write( gPtToGuiMsgFd, (char *)&msg, sizeof(SpPtGuiMsg) );

                }//end_validate_ch.

            }

            else if( SET_POWER == setPwrCh )

            {

                msg.msgCmd = SP_PROVISION_POWER_ADJUST;

 

                if( val >= 0 && val <= 4 )

                {

                    pCurrentSpPeer->txPowerLvlPrev = pACPNTmp->txPowerLvlCurr;

                    pCurrentSpPeer->txPowerLvlCurr = val;

 

                    if( 1 == nEnableACP )

                        write( gPtToGuiMsgFd, (char *)&msg, sizeof(SpPtGuiMsg) );

                }

            }

        }

        SDP_RELEASE

    }

RETURN_SET_CH_POWER:

    DCA_DATA_RELEASE( headDCA.semMutex )

 

    return;

}

/**

* @func:send scp command to let Ap to adjust tx power.(add case on AP sides too)

* @param:upOrDown  0(turn down), 1(turn up)

* @return: 0--no change; 1--changed; 2--change pending;

* @TODO: 1)add 5Ghz support. 2)add NDR support. 3)add tx power.

*/

void adjustSpPower( PAutoChannelAndPower pACPNode, int upOrDown )

{

    SpPtGuiMsg           msg;

 

    if( NULL == pACPNode )

        return;

 

    msg.msgCmd = SP_PROVISION_POWER_ADJUST;

    memcpy( msg.msgMac, pACPNode->BSSID, sizeof( pACPNode->BSSID ) );

    BSSID_TO_ID( msg.msgMac );

 

    //down or up

    if( TX_POWER_UP == upOrDown ) //txPower up

    {

        pACPNode->txPowerLvlPrev = pACPNode->txPowerLvlCurr;

        if( pACPNode->txPowerLvlCurr < 4 && pACPNode->txPowerLvlCurr > 0)

        {

            pACPNode->txPowerLvlCurr--;

            write( gPtToGuiMsgFd, (char *)&msg, sizeof(SpPtGuiMsg) );

        }

    }

    else if( TX_POWER_DOWN == upOrDown )//txpower down

    {

        pACPNode->txPowerLvlPrev = pACPNode->txPowerLvlCurr;

        if( pACPNode->txPowerLvlCurr < 4 && pACPNode->txPowerLvlCurr > 0)

        {

            pACPNode->txPowerLvlCurr++;

            write( gPtToGuiMsgFd, (char *)&msg, sizeof(SpPtGuiMsg) );

        }

    }

    return;

}

 

DSTATIC void freeColorScheme(void)

{

    DCA_DEBUG(1,("DCA:%s\n", __FUNCTION__));

    int row = 0;

    DCA_DATA_PROTECT( headDCA.semMutex );

 

    if( headDCA.s_colorScheme.pVertexColor != NULL )

    {

        SWFREE( headDCA.s_colorScheme.pVertexColor );

    }

 

    if( headDCA.pAPFixedCh != NULL )

    {

        for( row = 0; row < headDCA.s_colorScheme.numOfVertex; row++ )

            SWFREE( headDCA.pAPFixedCh[row] );

        SWFREE( headDCA.pAPFixedCh );

    }

#if INCLUDE_SONICPOINT_NDR

    if( headDCA.pAPFixedChNDR != NULL )

    {

        for( row = 0; row < headDCA.s_colorScheme.numOfVertex; row++ )

            SWFREE( headDCA.pAPFixedChNDR[row] );

        SWFREE( headDCA.pAPFixedChNDR );

    }

#endif

//     headDCA.s_colorScheme.kindOfScheme = 0;

    headDCA.s_colorScheme.numOfVertex = 0;

 

    DCA_DATA_RELEASE( headDCA.semMutex );

    return;

}

DSTATIC void memsetColorScheme(void)

{

    DCA_DEBUG(1,("DCA:%s\n", __FUNCTION__));

    int col = 0;

    DCA_DATA_PROTECT( headDCA.semMutex );

 

    if( headDCA.s_colorScheme.pVertexColor != NULL )

    {

        for( col = 0; col < headDCA.s_colorScheme.numOfVertex; col++ )

            headDCA.s_colorScheme.pVertexColor[col] = 0;

    }

    /*statistics of scheme*/

    memset( &headDCA.s_rfEvaluate, 0x00, sizeof(sRFEvaluate) );

    headDCA.s_colorScheme.schemeCnt = 0;

    headDCA.isNeedDoRfa             = 0;

    DCA_DATA_RELEASE( headDCA.semMutex );

    return;

}

 

/**

  *@NOTE: -1--failed 0--SW_ERR_SUCCESS.

*/

DSTATIC int setupColorScheme(void)

{

    DCA_DEBUG(1,("DCA:%s\n", __FUNCTION__));

    int row = 0;

//     int deltaNum = 0;

 

    DCA_DATA_PROTECT( headDCA.semMutex );

 

    do

    {

        if( 0 == headDCA.slList.count )

            break;

        if( headDCA.s_colorScheme.numOfVertex < headDCA.slList.count )

        {

            freeColorScheme();

//          headDCA.s_colorScheme.kindOfScheme      = NUM_KIND_COLOR_SCHEME;  /*@@@@@@@@@@@@@@*//*row*/

            headDCA.s_colorScheme.numOfVertex    = headDCA.slList.count;

            headDCA.s_colorScheme.schemeCnt         = 0;

 

            /*pVertexColor*/

            headDCA.s_colorScheme.pVertexColor = (unsigned char*) SWMALLOC( sizeof(unsigned char)* headDCA.s_colorScheme.numOfVertex );

            if( NULL == headDCA.s_colorScheme.pVertexColor )

            {

                CRITICAL(("DCA: Failed to allocate memory for pVertexColor!\n"));

                break;

            }

            memset( headDCA.s_colorScheme.pVertexColor, 0x00, headDCA.s_colorScheme.numOfVertex *sizeof(unsigned char) );

            /*pAPFixedCh*/

            headDCA.pAPFixedCh = (unsigned char **) SWMALLOC( sizeof( unsigned char* ) * headDCA.s_colorScheme.numOfVertex);

            if( NULL == headDCA.pAPFixedCh)

            {

                CRITICAL(("DCA: Failed to allocate memory for pAPFixedCh!\n"));

                break;

            }

 

            for( row = 0; row < headDCA.s_colorScheme.numOfVertex; row++ )

            {

                headDCA.pAPFixedCh[row] = (unsigned char *) SWMALLOC( sizeof(unsigned char)* 2 );    /*primary and secondary*/

                if( NULL == headDCA.pAPFixedCh[row] )

                {

                    CRITICAL(("DCA: Failed to allocate memory for pAPFixedCh!\n"));

                    break;

                }

                memset( headDCA.pAPFixedCh[row], 0x00 , 2 * sizeof(unsigned char) );  /*main and secondary*/

            }

#if INCLUDE_SONICPOINT_NDR

            /*pAPFixedChNDR*/

            headDCA.pAPFixedChNDR = (unsigned char **) SWMALLOC( sizeof( unsigned char* ) * headDCA.s_colorScheme.numOfVertex);

            if( NULL == headDCA.pAPFixedChNDR)

            {

                CRITICAL(("DCA: Failed to allocate memory for pAPFixedChNDR!\n"));

                break;

            }

 

            for( row = 0; row < headDCA.s_colorScheme.numOfVertex; row++)

            {

                headDCA.pAPFixedChNDR[row] = (unsigned char*) SWMALLOC( sizeof(unsigned char)* 2 );

                if( NULL == headDCA.pAPFixedChNDR[row] )

                {

                    CRITICAL(("DCA: Failed to allocate memory for pAPFixedChNDR!\n"));

                    break;

                }

                memset( headDCA.pAPFixedChNDR[row], 0x00 , 2 * sizeof(unsigned char) );  /*main and secondary*/

            }

#endif

        }

 

        DCA_DATA_RELEASE(headDCA.semMutex);

        return SW_ERR_SUCCESS;

    }while(0);

 

    DCA_DATA_RELEASE(headDCA.semMutex);

    return SW_ERR_FAIL;

}

 

DSTATIC void freeAdjacentAndRSSIMatrix(void)

{

    DCA_DEBUG(1,("DCA:%s\n", __FUNCTION__));

    int row = 0;

 

    DCA_DATA_PROTECT( headDCA.semMutex );

 

    if( headDCA.pAdjacentMatrix != NULL )      /*to be cautious, we check it first*/

    {

        DCA_DEBUG(1,("%s:headDCA.pAdjacentMatrix != NULL\n", __FUNCTION__));

        for(row = 0; row < headDCA.m_matrixLen; row++)

            SWFREE( headDCA.pAdjacentMatrix[row] );

        SWFREE( headDCA.pAdjacentMatrix );

    }

 

    if( headDCA.pRSSIMatrix != NULL )      /*to be cautious, we check it first*/

    {

        DCA_DEBUG(1,("%s:headDCA.pRSSIMatrix != NULL\n", __FUNCTION__));

        for(row = 0; row < headDCA.m_matrixLen; row++)

            SWFREE( headDCA.pRSSIMatrix[row] );

        SWFREE( headDCA.pRSSIMatrix );

    }

    headDCA.m_matrixLen = 0;

    DCA_DATA_RELEASE( headDCA.semMutex );

    return;

}

/**

* @func: allocate memory for matrix.

* @param:typeSize btyes occupied by the type.

*/

void ** setupMatrix( int dimension, int typeSize )

{

    DCA_DEBUG(1,("DCA[%s]\n", __FUNCTION__))

    void  **p = NULL;

    int row;

 

    do

    {

        p = (void **) SWMALLOC( sizeof(p) * dimension );

        if( NULL == p )

        {

            CRITICAL(("DCA:Failed to allcate memory for matrix!\n"));

            break;

        }

   

        for( row = 0; row < dimension; row++)

        {

            p[row] = (void*) SWMALLOC(  dimension * typeSize );

 

            if( NULL == p[row] )

            {

                CRITICAL(("DCA:Failed to allcate memory for matrix!\n"));

                break;

            }

            memset( p[row], 0x00, dimension *typeSize );

        }

        return p;

    }while(0);

    /* error process */

    if( p != NULL)

    {

        for( row = 0; row < dimension; row++ )

            SWFREE( p[row] );

        SWFREE(p);

    }

    return NULL;

}

 

/**

*  @func:   crete the adjacency Matrix and RSSI Matrix based on the number of connected AP

*  @NOTE:   (1)check pointer   (2)-1--failed    0--sucessful.

*           (3) we shouldn't free the allocated memory as scanning is based on time.

*/

DSTATIC int setupAdjacentAndRSSIMatrix()

{

    DCA_DEBUG(1,("%s\n", __FUNCTION__));

 

    DCA_DATA_PROTECT( headDCA.semMutex );

 

    /*initilize the threshold of rssi*/

    headDCA.m_rssiCurThreshold = RSSI_THRESHOLD_MINI;

 

    if( headDCA.slList.count )  /*due to 'mz_buf_allco'*/

    {

        if( 0 == headDCA.m_matrixLen )

        {

            headDCA.m_matrixLen = headDCA.slList.count; //getMaxSonicPoints();

            headDCA.m_oldListCount = headDCA.slList.count;

        }

        do

        {

            if( NULL == headDCA.pAdjacentMatrix )

            {

                headDCA.pAdjacentMatrix = (unsigned char **) setupMatrix( headDCA.m_matrixLen, sizeof(unsigned char) );

                if( NULL == headDCA.pAdjacentMatrix )

                {

                    CRITICAL(("DCA:Failed to allocate memory for matrix!\n"))

                    break;

                }

            }

 

            if( NULL == headDCA.pRSSIMatrix )

            {

                headDCA.pRSSIMatrix     = (unsigned long **) setupMatrix( headDCA.m_matrixLen, sizeof(unsigned long) );

                if( NULL == headDCA.pRSSIMatrix )

                {

                    CRITICAL(("DCA:Failed to allocate memory for matrix!\n"))

                    break;

                }

            }

 

            DCA_DATA_RELEASE( headDCA.semMutex );

            return SW_ERR_SUCCESS;

        }while(0);

    }

 

    DCA_DATA_RELEASE( headDCA.semMutex );

    return SW_ERR_FAIL;

 

}

DSTATIC void memsetRSSIMatrix(void)

{

    DCA_DEBUG(1,("DCA:%s\n", __FUNCTION__));

    int row = 0, col = 0;

 

    DCA_DATA_PROTECT( headDCA.semMutex );

 

    if( headDCA.pRSSIMatrix != NULL )

    {

        for(row = 0; row < headDCA.m_matrixLen; row++)

        {

            if( headDCA.pRSSIMatrix[row] != NULL)

            {

                for(col = 0; col < headDCA.m_matrixLen; col++)

                    headDCA.pRSSIMatrix[row][col] = 0;

            }

        }

    }

    DCA_DATA_RELEASE( headDCA.semMutex );

    return;

}

DSTATIC void memsetAdjacentMatrix(void)

{

    DCA_DEBUG(1,("DCA:%s\n", __FUNCTION__));

    int row = 0, col = 0;

 

    DCA_DATA_PROTECT( headDCA.semMutex );

 

    if( headDCA.pAdjacentMatrix != NULL )

    {

        for(row = 0; row < headDCA.m_matrixLen; row++)

        {

            if( headDCA.pAdjacentMatrix[row] != NULL)

            {

                for(col = 0; col < headDCA.m_matrixLen; col++)

                    headDCA.pAdjacentMatrix[row][col] = 0;

            }

        }

    }

    DCA_DATA_RELEASE( headDCA.semMutex );

    return;

}

/**

* @func:

* @param:pAdjacent if NULL, just move valid data backward in matrix.[backward not forward since new sp has been insert from the head of list.]

* @param:pRSSI if NULL, just move valid data backward in matrix.

*/

DSTATIC void moveBackwardMatrix( int oldMatrixLen, int newMatrixLen, unsigned char ** pAdjacent, unsigned long **pRSSI )

{

    DCA_DEBUG(1,("DCA[%s]\n", __FUNCTION__))

    int row = 0, col = 0, i = 0, flag = 0;

    int rowNew = 0, colNew = 0;

    int delCount = 0;

 

    if( 0 == headDCA.slList.count )

        return;

 

    delCount = headDCA.s_InfoUpdateMatrix.delCount;

 

    for( row = oldMatrixLen-1, rowNew = newMatrixLen-1; row >= 0 && rowNew >= 0; row-- )

    {

        flag = 0;

        for( i = 0; i < delCount; i++ )

        {

            if( row == headDCA.s_InfoUpdateMatrix.delIndexArray[i] )

            {

                flag = 1;

                break;

            }

        }

        if( 1 == flag )

        {

            /*set invalid data cell to '0'.*/

            memset( headDCA.pAdjacentMatrix[row], 0x00, oldMatrixLen );

            memset( headDCA.pRSSIMatrix[row], 0x00, oldMatrixLen );

            continue;   /*find if row is in indexArray[i]*/

        }

 

        for( col = oldMatrixLen-1, colNew = newMatrixLen-1; col >= 0 && colNew >= 0; col-- )

        {

            flag = 0;

            for( i = 0; i < delCount; i++ )

            {

                if( col == headDCA.s_InfoUpdateMatrix.delIndexArray[i] )

                {

                    flag = 1;

                    break;

                }

            }

            if( 1 == flag )

            {

                /*set invalid data cell to '0'.*/

                headDCA.pAdjacentMatrix[row][col] = 0;

                headDCA.pRSSIMatrix[row][col] = 0;

                continue;   /*jump over invalid column.*/

            }

 

            if( NULL == pAdjacent )

            {

                headDCA.pAdjacentMatrix[rowNew][colNew] = headDCA.pAdjacentMatrix[row][col];

                headDCA.pAdjacentMatrix[row][col] = 0;  /*need to clear the value.(important)*/

            }

            else

                headDCA.pAdjacentMatrix[rowNew][colNew] = pAdjacent[row][col];  /*old*/

 

            if( NULL == pRSSI )

            {

                headDCA.pRSSIMatrix[rowNew][colNew] = headDCA.pRSSIMatrix[row][col];

                headDCA.pRSSIMatrix[row][col] = 0;  /*need to clear the value.(important)*/

            }

            else

                headDCA.pRSSIMatrix[rowNew][colNew] = pRSSI[row][col];

 

            colNew--;

        }

        rowNew--;

    }

}

/**

* @func:update adjacent Matrix and RSSI matrix.

* @return DCA_NO_NODE

* @NOTE:1)called when the state of APs is stable to avoid calling this function too frequently.

        2)m_matrixLen == length of memory. valid length = headDCA.slList.count.

*/

int updateAdjacentAndRSSIMatrix()

{

    DCA_DEBUG(1,("DCA:%s\n", __FUNCTION__));

    int row = 0;

    int matrixLenOld = 0, oldListcount = 0;

    unsigned char **pAdjacent = NULL;

    unsigned long **pRSSI = NULL;

 

    DCA_DATA_PROTECT( headDCA.semMutex );

 

    if( 0 == headDCA.m_matrixLen )

    {

        /*setup the matrix at the first time as the task has just started.*/

        DCA_DEBUG(1,("DCA[%s] setup the matrix at the first time as the task has just started\n", __FUNCTION__))

        if( SW_ERR_FAIL == setupAdjacentAndRSSIMatrix() )

        {

            CRITICAL(("DCA: Failed to setup matrix!\n"));

            DCA_DATA_RELEASE( headDCA.semMutex );

            return SW_ERR_FAIL;

        }

    }

    else if(  headDCA.slList.count <= headDCA.m_matrixLen )

    {

        /*Just move RSSI and Adjacent matrix forward as it's not necessary to reallocate memory under this condition.*/

        /*copy the valid data back exclude indexArray[i]. (move rowNew and colNew)*/

        DCA_DEBUG(1,("DCA[%s] headDCA.slList.count <= headDCA.m_matrixLen \n", __FUNCTION__))

        moveBackwardMatrix( headDCA.m_oldListCount, headDCA.slList.count, NULL, NULL );

    }

    else /*headDCA.slList.count > headDCA.m_matrixLen */

    {

        DCA_DEBUG(1,("DCA[%s] headDCA.slList.count > headDCA.m_matrixLen \n", __FUNCTION__))

        /*it has executed 'delete sp' or 'add sp' operation.*/

        matrixLenOld = headDCA.m_matrixLen;

        oldListcount = headDCA.m_oldListCount;

        pAdjacent = headDCA.pAdjacentMatrix;

        pRSSI = headDCA.pRSSIMatrix;

 

        headDCA.m_matrixLen = 0;            /*To get new m_matrixLen*/

        headDCA.pAdjacentMatrix = NULL;     /*To allocate new memory*/

        headDCA.pRSSIMatrix = NULL;

 

        /*reallocated the memory for matrix*/

        if( SW_ERR_FAIL == setupAdjacentAndRSSIMatrix() )

        {

            CRITICAL(("DCA: Failed to setup matrix!\n"));

            DCA_DATA_RELEASE( headDCA.semMutex );

            return SW_ERR_FAIL;

        }

 

        DCA_DEBUG(1,("matrixLenOld = %d\n", matrixLenOld))

        DCA_DEBUG(1,("headDCA.m_oldListCount = %d\n", headDCA.m_oldListCount))

        moveBackwardMatrix( oldListcount, headDCA.slList.count, pAdjacent, pRSSI );

        /*free old matrix*/

        DCA_DEBUG(1,("DCA[%s:%d] Free old matrix!\n",__FUNCTION__, __LINE__))

        if( pAdjacent != NULL)

        {

            for( row = 0; row < matrixLenOld; row++ )

            {

                SWFREE( pAdjacent[row] );

            }

            SWFREE( pAdjacent );

        }

 

        if( pRSSI != NULL )

        {

            for( row = 0; row < matrixLenOld; row++ )

            {

                SWFREE( pRSSI[row] );

            }

            SWFREE( pRSSI );

        }

    }

    /* clear update matrix info.*/

    DCA_DEBUG(1,("DCA[%s] clear update matrix info \n", __FUNCTION__))

    memset( &headDCA.s_InfoUpdateMatrix, 0x00, sizeof(sInfoUpdateMatrix) );

    headDCA.m_oldListCount = headDCA.slList.count;  /*update oldListCount*/

 

    DCA_DATA_RELEASE( headDCA.semMutex );

    return SW_ERR_SUCCESS;

}

/**

* @func: update AP node info or send command to calcTask to start calculation.

*/

SwErrCode updateSPInfo( const SpPtGuiMsg *pMsg )

{

    int n = 0, m_startCalcFlag = ACP2CALC_START_CALC_CMD;

    unsigned char           mac[6]      = {'\0' };

    PAutoChannelAndPower    pACPNode    = NULL;

 

    DCA_DATA_PROTECT( headDCA.semMutex )

 

    memcpy( mac, pMsg->msgMac, 6 );

    ID_TO_BSSID( mac );

 

    if( SP_ADD_NEW_ONE == pMsg->msgCmd )

    {

        DCA_DEBUG( 1, ("DCA[ %s ]: SP_ADD_NEW_ONE\n", __FUNCTION__) )

        /* add new when added by sdp.To be reliable, we need to check if it exists already!*/

        pACPNode = ( PAutoChannelAndPower ) findSpByMAC( mac, NULL );

 

        if( NULL == pACPNode )

        {

            pACPNode = ( PAutoChannelAndPower ) SWMALLOC( sizeof(AutoChannelAndPower) );

            if( NULL == pACPNode )

            {

                CRITICAL(("DCA[ %s ]: Failed to allocate memory!", __FUNCTION__ ));

                DCA_DATA_RELEASE( headDCA.semMutex );

                return SW_ERR_FAIL;

            }

            /*get BSSID*/

            memset( pACPNode, 0x00, sizeof(pACPNode) );

            memcpy( pACPNode->BSSID, mac, sizeof(pACPNode->BSSID) );

            /*check if it's set to fixed channel.*/

            //...

            slListAdd( &headDCA.slList, ( SlListNode* )pACPNode );

 

            headDCA.enum_spScanMode = SCANMOED_ALL_SP_SCAN_BASED_ON_TIME;

            m_justBoot = 0;

            m_channelAssignFlag = 1;

            headDCA.s_InfoUpdateMatrix.addCount++;  /*keep record*/

        }

    }

    else if( SP_DEL_OLD_ONE == pMsg->msgCmd )

    {

        DCA_DEBUG( 1, ("DCA[ %s ]: SP_DEL_OLD_ONE\n", __FUNCTION__ ) )

        /* deleted by sdp*/

        pACPNode = ( PAutoChannelAndPower ) findSpByMAC( mac, &headDCA.s_InfoUpdateMatrix.delIndexArray[ headDCA.s_InfoUpdateMatrix.validIndex++ ]);

        if( pACPNode != NULL )

        {

            slListDelete( &headDCA.slList, ( SlListNode * )pACPNode );

            SWFREE( pACPNode );

 

            m_justBoot = 0;

//          m_channelAssignFlag = 1;

            m_channelAssignFlag = 0;       /*do nothing when delete a sp*/

 

            headDCA.s_InfoUpdateMatrix.delCount++;  /*keep record*/

        }

    }

    else if( SP_START_AUTO_CHANNEL == pMsg->msgCmd )

    {

        DCA_DEBUG( 1, ("DCA[ %s ]: GUI send 'calc' command ...\n", __FUNCTION__ ) )

        m_justBoot = 0;

        m_channelAssignFlag = 0;

 

        headDCA.enum_spScanMode = gDCASchedParam.dcaScanMode;   /*decided by configuration*/

        DCA_DEBUG(1,("DCA[ %s ]: Force all sp to scan!\n", __FUNCTION__ ))

 

        if( (n = write( gCalcTaskPipeFd, (char *)&m_startCalcFlag, sizeof(int))) != sizeof(int) )

        {

            CRITICAL(("DCA: Failed to write pipe[ gCalcTaskPipeFd ]!"))

            DCA_DATA_RELEASE( headDCA.semMutex );

            return SW_ERR_FAIL;

        }

    }

    else if( SP_CHANGE_INTO_OPERATIONAL == pMsg->msgCmd )

    {

        DCA_DEBUG(1,("DCA[ %s ] SP = "MAC_STR_FORMAT" change into operational!\n", __FUNCTION__,MAC_ARRARY(pMsg->msgMac)) )

        pACPNode = ( PAutoChannelAndPower ) findSpByMAC( mac, NULL );

 

        /*scanning--->operational: exclude normal scanning operation(add new member in peerList).*/

        if( pACPNode != NULL )

        {

            pACPNode->enum_spScanMode = SCANMODE_FORCE_THIS_SP_SCAN;

            m_channelAssignFlag = 1;    /*wait latency time before starting calcTask.*/

        }

    }

#if DEBUGALL

    else

    {

        DCA_DEBUG( 1, ("DCA[ %s ]: Unknown command!\n", __FUNCTION__ ) )

    }

#endif

    DCA_DATA_RELEASE( headDCA.semMutex )

    return SW_ERR_SUCCESS;

}

 

/**

 * @ initialize sp info.

 */

int spInfoInit()

{

    /*initialization of headDCA*/

    slListInit(&headDCA.slList);

 

    headDCA.m_rssiCurThreshold = RSSI_THRESHOLD_MINI;

    PSpPeerList pCurrent = NULL;

    PAutoChannelAndPower pACPNode = NULL;

 

    SDP_PROTECT

    pCurrent = (PSpPeerList)(gSonicPointSlList.slHead.next);

    while(pCurrent)

    {

        pACPNode = (PAutoChannelAndPower ) SWMALLOC( sizeof(AutoChannelAndPower) ); /* malloc?*/

        if( NULL == pACPNode)

        {

            CRITICAL(("DCA:Fail to allocate memory!" ));

            return SW_ERR_FAIL;

        }

        /*get BSSID*/

        memset( pACPNode, 0x00, sizeof(pACPNode) );

        memcpy( pACPNode->BSSID, pCurrent->spPrvCfg.id, SONICPOINT_BSSID_LENGTH);

        pACPNode->BSSID[5] += 1;  /*BSSID = id+1*/

        /*set sp scan based on time.*/

        /*headDCA.enum_spScanMode=SCANMOED_ALL_SP_SCAN_BASED_ON_TIME

            --> pACPNode->enum_spScanMode...

          headDCA.enum_spScanMode !=SCANMOED_ALL_SP_SCAN_BASED_ON_TIME

            -->all sp scan

        */

        pACPNode->enum_spScanMode = SCANMODE_THIS_SP_SCAN_BASED_ON_TIME;

 

        slListAdd( &headDCA.slList, (SlListNode*)pACPNode);

        /*intilize some member*/

        pCurrent = pCurrent->spNext;

    }

    SDP_RELEASE;

 

    /* create necessary pipe*/

    /*pipe: add spPeer <----> ACPTask*/

    if ( pipeDevCreate ( ACP_DYNAMIC_CHANNEL_PIPE, ACPTASK_PIPE_MAX_MSGS, ACPTASK_PIPE_UNIT ) == 0 )

    {

        gDynamicChannelFd = open( ACP_DYNAMIC_CHANNEL_PIPE, O_RDWR , 0666);

    }

    else

    {

        CRITICAL(("DCA:Fail to create pipe!\n"));

        return SW_ERR_FAIL;

    }

    /*pipe: ACPTask <---> calcTask  SSPP_SCAN <---> calcTask

        1.used to terminate calcTask since it doesn't make sense when new Aps join.

        2.used to notify the calcTask that the scan result has been returned.

    */

    if ( pipeDevCreate ( CALCTASK_PIPE, CALCTASK_PIPE_MAX_MSGS, CALCTASK_PIPE_UNIT )  == 0 )

    {

        gCalcTaskPipeFd = open( CALCTASK_PIPE, O_RDWR  , 0666);

    }

    else

    {

        CRITICAL(("DCA:Fail to create pipe!\n"));

        return SW_ERR_FAIL;

    }

 

    DCA_DATA_PROTECT(sACPFlag.semMutex)

    sACPFlag.m_calcTaskRunning = 0;

    sACPFlag.m_acpTaskRunning  = 1; //????      /*set flag so that sdpAddPeerBySDP(del) can write msg to acpTask*/

    DCA_DATA_RELEASE(sACPFlag.semMutex)

    return 0;

}

 

DSTATIC void calculateChTaskExit()

{

    DCA_DEBUG(1,("%s [%d]receive a signal\n", __FUNCTION__, __LINE__ ))

    /*check if we need to exit critical area*/

    return;

}

 

/**

* @func:   check pipes gCalcTaskPipeFd.

* @param:timeValue  timeOut for select.

* @return: 0 --need to check command and the count of command.

           1 --return due to timeout.

*/

DSTATIC int checkCalcTaskPipe(int timeValue)

{

    struct timeval  timeOut;

    fd_set          readFds;

    int             n = gCalcTaskPipeFd, ret = 0;

    int             i = 0;

 

    FD_ZERO( &readFds );

    FD_SET( gCalcTaskPipeFd, &readFds);

    /*timeOut*/

    timeOut.tv_sec  = timeValue;       /*6s*/

    timeOut.tv_usec = 0;

    ret = select( n + 1, &readFds, NULL, NULL, &timeOut);

 

    if( ret <= 0)

    {

        DCA_DEBUG(1,("DCA[%s]: select error or timeout!\n", __FUNCTION__));

        return ret;

    }

 

    if( FD_ISSET ( gCalcTaskPipeFd, &readFds ) )

    {

        /*only check if it is the cmd to stop calculation.*/

        DCA_DEBUG(1,("DCA[%s:%d]:  pipe[gCalcTaskPipeFd] needs to read!\n", __FUNCTION__, __LINE__));

        /* pipe: ACPTask <---> calcTask */

//      n = ioctl(acpTaskToCalcTaskFd, FIONMSGS, 0);

        memset( &s_calcTaskPipeCmdCnt, 0x00, sizeof(s_calcTaskPipeCmdCnt) ); /*initialize*/

        memset( m_calcCmdArray, 0x00 , sizeof(m_calcCmdArray) );

        /*try to read all msgs in the pipe*/

        msgLen = read( gCalcTaskPipeFd, (char *)m_calcCmdArray, CALCTASK_PIPE_MAX_MSGS);

 

        for(i = 0; i < msgLen/CALCTASK_PIPE_UNIT; i++)

        {

            if( ACP2CALC_START_CALC_CMD == m_calcCmdArray[i])

            {

                DCA_DEBUG(1,("DCA[%s]:  read pipe => ACP2CALC_START_CALC_CMD!\n", __FUNCTION__));

                /*it just needs to check ACP2CALC_START_CALC_CMD*/

                s_calcTaskPipeCmdCnt.m_startCalcCmdCnt++;

            }

            else if( ACP2CALC_STOP_CALC_CMD == m_calcCmdArray[i])

            {

                DCA_DEBUG(1,("DCA:  read pipe <= ACP2CALC_STOP_CALC_CMD!\n", __FUNCTION__));

                s_calcTaskPipeCmdCnt.m_stopCalcCmdCnt++;

            }

            else if( SSPPSCAN_RETURN ==  m_calcCmdArray[i])

            {

                DCA_DEBUG(1,("DCA:  read pipe <= SSPPSCAN_RETURN!\n",__FUNCTION__));

                s_calcTaskPipeCmdCnt.m_ssppScanReturn++;

            }

        }

    }//end_check_gCalcTaskPipeFd

    return ret;

}

/**

* @func:        check if all Ap are ready, if yes , then we do dynamic channel assignment.

* @return:      AP_STATES-enum

* @exception:   (1) we need to wait until they are ready if the workstates are as follows:

*                   reboot, provisioning, unprovisioned

*/

DSTATIC int checkAPOk()

{

    /*the maximum loop of checking Ap state,since it maybe in the rebooting all the  time.*/

    int m_maxCheckCnt = 30, ret = 0;

    int m_APWorkStateStat[SP_STATE_MAX] = {0};

#if DEBUGALL

    int i = 0;

    char *p_stateName[20] = {

        "SP_STATE_SAFEMODE             (0)", "SP_STATE_UNPROVISIONED        (1)", "SP_STATE_PROVISIONING         (2)",

        "SP_STATE_OPERATIONAL          (3)", "SP_STATE_UNRESPONSE           (4)", "SP_STATE_UPDATINGFIRMWARE     (5)",

        "SP_STATE_INITILIZING          (6)", "SP_STATE_OVERLIMIT            (7)", "SP_STATE_REBOOTING            (8)",

        "SP_STATE_FAILEDPROVISION      (9)", "SP_STATE_FAILEDFIRMWARE       (10)", "SP_STATE_SCANNING             (11)",

        "SP_STATE_MANUFACTURING        (12)",

        /* in order to sync with manufacturing, new states will add after MANUFACTURING */

        "SP_STATE_DISABLE              (13)", "SP_STATE_DOWNLOADING_FIRMWARE (14)", "SP_STATE_NOFIRMWARE           (15)",

        "SP_STATE_FAILEDSCAN           (16)", "SP_STATE_WIDP                 (17)", "SP_STATE_WIDP_TO              (18)",

        "SP_STATE_WRITING_FIRMWARE     (19)",

    };

#endif

    int m_waitAPFlag = 1; /*if it needs to wait ap until it enters into operational mode.*/

 

    /* if all Aps are in operational mode, we start calculating immediately.*/

    while( (1 == m_waitAPFlag) && m_maxCheckCnt )

    {

        memset( m_APWorkStateStat, 0x00, sizeof(m_APWorkStateStat) );

 

        SDP_PROTECT

 

        if( 0 == gSonicPointSlList.count )

        {

            SDP_RELEASE

            return NO_AP_CONNECTED;

        }

 

        PSpPeerList pCurrentSpPeer = ( PSpPeerList ) gSonicPointSlList.slHead.next;

        while( NULL != pCurrentSpPeer )

        {

            m_APWorkStateStat[pCurrentSpPeer->spStatus.spsWorkState]++;

            pCurrentSpPeer = pCurrentSpPeer->spNext;

        }

#if DEBUGALL

        DCA_DEBUG(1,("DCA[%s]: the statistics of SP are as follows:\n", __FUNCTION__))

        DCA_DEBUG(1,("---------------------------------------------------------\n"))

        for(i = 0; i < SP_STATE_MAX; i++)

            DCA_DEBUG(1,("m_APWorkStateStat[ %-s ] = %d\n", p_stateName[i], m_APWorkStateStat[i]))

        DCA_DEBUG(1,("---------------------------------------------------------\n"))

#endif

        /*all in opearational mode?*/

        if( m_APWorkStateStat[SP_STATE_OPERATIONAL] == gSonicPointSlList.count )

        {

            SDP_RELEASE

            DCA_DEBUG(1,("DCA:[%s]  AP_IS_STABLE!\n", __FUNCTION__))

            return AP_IS_STABLE; /*work state of all aps are stable now*/

        }

 

        SDP_RELEASE

 

        /*check pipe gCalcTaskPipeFd to find if there is a command which stops calculating.*/

        ret = checkCalcTaskPipe( 10 );

        if( ret > 0 )

        {

            /*there is a command coming in.But we just care about ACP2CALC_STOP_CALC_CMD command.*/

            s_calcTaskPipeCmdCnt.m_startCalcCmdCnt  = 0;

            s_calcTaskPipeCmdCnt.m_ssppScanReturn   = 0;

            if( s_calcTaskPipeCmdCnt.m_stopCalcCmdCnt > 0 )

            {

                /*process before exit if necessary*/

                DCA_DEBUG(1,("DCA:[%s]  STOP_CALC!\n", __FUNCTION__));

                return STOP_CALC; /*ACPTask need calcTask to stop calculating*/

            }

        }

        /*we need to wati under some work states */

        if( m_APWorkStateStat[SP_STATE_UNPROVISIONED]    || m_APWorkStateStat[SP_STATE_PROVISIONING]         ||\

            m_APWorkStateStat[SP_STATE_UPDATINGFIRMWARE] || m_APWorkStateStat[SP_STATE_REBOOTING]            ||\

            m_APWorkStateStat[SP_STATE_SCANNING]         || m_APWorkStateStat[SP_STATE_DOWNLOADING_FIRMWARE] ||\

            m_APWorkStateStat[SP_STATE_WRITING_FIRMWARE]

         )

        {

            m_waitAPFlag = 1;

            m_maxCheckCnt --;   /*to wait more times:maxTime= m_maxCheckCnt *checkCalcTaskPipeTime = 300s*/

        }

        else

        {

            m_waitAPFlag = 0;

            DCA_DEBUG(1,("DCA:[%s]  NO_NEED_TO_WAIT!\n", __FUNCTION__));

            return NO_NEED_TO_WAIT;   /*we don't need to wait when aps work in some work states*/

        }

    }

    DCA_DEBUG(1,("DCA:[%s]  WAIT_AP_TO_STABLE_TIMEOUNT!\n", __FUNCTION__));

    return WAIT_AP_TO_STABLE_TIMEOUNT;  /*time's up when we check if all aps are in stable mode*/

}

 

DSTATIC int isColorOk( int vertex, int color)

{

    DCA_DEBUG(1,("isColorOk start\n"))

    int m_vIndex = 0;                                   /*the index for vertex*/

    for( m_vIndex = 0; m_vIndex < vertex; m_vIndex++)  /* for 0 to (t-1)*/

    {

        if( headDCA.pAdjacentMatrix[vertex][m_vIndex] && color == headDCA.s_colorScheme.pVertexColor[m_vIndex] )

        {

            DCA_DEBUG(1,("isColorOk == not ok end\n"))

            return 0;

        }

    }

    DCA_DEBUG(1,("isColorOk == ok end\n"))

    return 1;

}

#if 1

/**

* @func: take the effection of APs assigned dynamic Channels into account when do rfa for APS  without assigned channels.

*/

DSTATIC void doRfa( PAutoChannelAndPower pACPNode, int position, sACPRfa *pACPRfa)

{

    /*take the node from headDCA to the node before pACPNode*/

    PAutoChannelAndPower pACPNodeTemp = ( PAutoChannelAndPower ) headDCA.slList.slHead.next;

    int pos = 0, i = 0, coef = 0;

 

    if( NULL == pACPNode || NULL == pACPRfa)

        return;

 

    memcpy( pACPRfa, &pACPNode->s_rfa, sizeof(sACPRfa) );

 

    while( pos < position && pACPNodeTemp != pACPNode )

    {

        if( pACPNode->s_radioInfo[1].ch[0] != 0 )   /*fixed channel.*/

        {

            /*fixed channel and on optional channel*/

            memset( pACPNode->optionalCh, 0x00, sizeof( pACPNode->optionalCh )/sizeof( unsigned char ) );

            break;

        }

        else

        {

            for( i = 1; i <= 11; i++ )//just index

            {

                /*Aps assigned a dynamic channels.*/

                /*dynamicChAssign[1][0] has no value currently.*/

//                 if( pACPNodeTemp->dynamicChAssign[1][0] >= pACPRfa->rfaCh[i] && pACPNodeTemp->dynamicChAssign[1][0] <= ( pACPRfa->rfaCh[i]+4 ) )

                printf("headDCA.s_colorScheme.pVertexColor[%d] = %d\n", pos, headDCA.s_colorScheme.pVertexColor[pos]);

                if( abs(headDCA.s_colorScheme.pVertexColor[pos] - pACPRfa->rfaCh[i]) <= 4 )

                {

                    /*take channels related to channel i into consideration.*/

//                     coef = rfaGetInterferenceCoefChan( pACPRfa->rfaCh[i], pACPNodeTemp->dynamicChAssign[1][0] );

                    coef = rfaGetInterferenceCoefChan( pACPRfa->rfaCh[i], headDCA.s_colorScheme.pVertexColor[pos] );

                    pACPRfa->rfScore[i] = rfaOverflowCheckAdd( pACPRfa->rfScore[i], headDCA.pRSSIMatrix[position][pos]/coef );

                    DCA_DEBUG(1,("coef = %d \n",coef))

                    DCA_DEBUG(1,("headDCA.pRSSIMatrix[%d][%d] = %d\n", position, pos, headDCA.pRSSIMatrix[position][pos]))

                    DCA_DEBUG(1,("pACPRfa->rfScore[%d] = %d\n", pACPRfa->rfaCh[i], pACPRfa->rfScore[i]))

                }

                printf("i = %d\n", i);

            }

        }

        pos++;

        pACPNodeTemp = pACPNodeTemp->next;

    }

    /*print result*/

#if DEBUGALL

    DCA_DEBUG(1,("old rfa:\n"))

    for( i = 1; i <= 11; i++ )

    {

        DCA_DEBUG(1,("pACPNode->s_rfa.rfScore[%d] = %d\n", pACPNode->s_rfa.rfaCh[i], pACPNode->s_rfa.rfScore[i]));

    }

    DCA_DEBUG(1,("new rfa:\n"))

    for( i = 1; i <= 11; i++ )

    {

        DCA_DEBUG(1,("pACPRfa->rfScore[%d] = %d\n", pACPRfa->rfaCh[i], pACPRfa->rfScore[i]));

    }

#endif

 

    if( pos > 0 )   /* Don't do sort for the first node as it has already been done.*/

    {

        /*decide the order of channels to be asssigned based on RFA.*/

        decideOptionalCh( pACPNode, pACPRfa );

    }

}

#endif

/**

  *@func: find out the one appropriate result for dynmaic channel assignment under some constraints.

  *@param:vertex

*/

DSTATIC void giveOneAssign( PAutoChannelAndPower pACPNode, int vertex )

{

    int colorIndex = 0, i = 0;

 

    memset( &s_rfaTemp, 0x00, sizeof(s_rfaTemp) );

    DCA_DEBUG(1,("vertex=%d, headDCA.slList.count= %d\n", vertex, headDCA.slList.count))

 

    if( vertex >= headDCA.slList.count)

    {

#if DEBUGALL

        /*all node has been assigned sucessfully.*/

        DCA_DEBUG(1,("DCA[%s] all node has been assigned sucessfully and listed as follows:\n", __FUNCTION__));

        for( i = 0; i < headDCA.slList.count; i++)

        {

            DCA_DEBUG(1,("headDCA.s_colorScheme.pVertexColor[ %d ] = %d\n", i, headDCA.s_colorScheme.pVertexColor[i]));

        }

        DCA_DEBUG(1,("\n"));

#endif

        headDCA.s_colorScheme.schemeCnt++;

        return;

    }

    else

    {

        if( NULL == pACPNode )

        {

            DCA_DEBUG(1,("DCA[%s] pACPNode has been already NULL!\n"))

            return;

        }

 

        if( vertex > 0 && pACPNode->nEnableDCA && SP_STATE_OPERATIONAL == pACPNode->spsWorkState)

        {

            /*take the effect of APs asssigned channels into account.*/

            DCA_DEBUG(1,("DCA[ %s:%d ] do Rfa!\n", __FUNCTION__, __LINE__))

            doRfa( pACPNode, vertex , &s_rfaTemp);                              /* we shouldn't revise the pACPNode->s_rfaTemp since more attempt will be made.2012/9/21*/

        }

        for( colorIndex = 0; colorIndex < OPTIONAL_CHANNEL_NUM; colorIndex++)  /*3 channels*/

        {

            if( headDCA.s_colorScheme.schemeCnt > 0 )

                break;

 

            if( pACPNode->nEnableDCA && SP_STATE_OPERATIONAL == pACPNode->spsWorkState )                /*ignore the unresponsive AP.*/

            {

                if( SSPP_RETURN_OK == pACPNode->m_scanReturn )                  /*ignore the AP without response.*/

                {

                    if( 1 == isColorOk( vertex , pACPNode->optionalCh[colorIndex] ) )

                    {

                        DCA_DEBUG(1,("ok\n"))

                        headDCA.s_colorScheme.pVertexColor[vertex] = pACPNode->optionalCh[colorIndex];

                        DCA_DEBUG(1,("enter next.....\n"))

 

                        giveOneAssign( pACPNode->next, vertex+1 );

                    }

                }

                else

                {

                    /*the scanning result hasn't been returned sucessfully and we needs to jump over this vertex, or it'll get lost in dead loop.*/

                    headDCA.s_colorScheme.pVertexColor[vertex] = 0xff;

                    giveOneAssign( pACPNode->next, vertex+1 );

                    break;      /*it makes no sense to try other colorIndex, so it just return here*/

                }

            }

            else

            {

                /*jump over the AP with no response.*/

                headDCA.s_colorScheme.pVertexColor[vertex] = 0xff;

                giveOneAssign( pACPNode->next, vertex+1 );

                break;      /*it makes no sense to try other colorIndex, so it just return here*/

            }

        }

    }

    return;

}

#if 0

/**

  *@func: find out the all probable result for dynmaic channel assignment under ideal conditions.

  *@param:vertex

*/

DSTATIC void giveIdealAssign(int vertex)

{

    DCA_DEBUG(1,("DCA:[backTrack Matrix] vertex = %d  ...\n", vertex));

    int colorIndex = 0, i = 0;

    if( vertex >= headDCA.slList.count)

    {

        DCA_DEBUG(1,("DCA:[%s] the probable assignment as follows:\n", __FUNCTION__));

        for( i = 0; i < headDCA.slList.count; i++)

        {

            DCA_DEBUG(1,("%d ", headDCA.s_colorScheme.pVertexColor[i]));

 

            /*save the probable assignment*/

            /*we need to check if the index of scheme to be saved is greater than maximum scheme which can be saved*/

            if( headDCA.s_colorScheme.schemeCnt < headDCA.s_colorScheme.kindOfScheme)   /*Had been crashed here*/

            {

                headDCA.s_colorScheme.pAssignment[ headDCA.s_colorScheme.schemeCnt ][i] = headDCA.s_colorScheme.pVertexColor[i];

 

                DCA_DEBUG(1,("[%d] ", headDCA.s_colorScheme.pAssignment[ headDCA.s_colorScheme.schemeCnt ][i]));

            }

        }

        headDCA.s_colorScheme.schemeCnt++;

        DCA_DEBUG(1,("\n"));

        return;

    }

    else

    {

        for( colorIndex = 0; colorIndex < sizeof(channel)/sizeof(int); colorIndex++)  /*3 channels*/

        {

            DCA_DEBUG(1,("color[%d]=%d\n", colorIndex, channel[colorIndex]));

            if( isColorOk( vertex , channel[colorIndex] ) )

            {

                headDCA.s_colorScheme.pVertexColor[vertex] = channel[colorIndex];

                giveIdealAssign( vertex+1 );

            }

        }

        headDCA.s_colorScheme.pVertexColor[vertex]=0;

    }

    return;

}

#endif

/**

*  @func:transform a unsymmetrical matrix into a symmetrical matrix.

*  @param:p points to a matrix

*  @exampe: caused by failing to scan some managed AP due to short beacon Interval.

*      | 0 1 | ---> | 0 1 |

*      | 0 0 | ---> | 1 0 |

* @return:  1--the matrix is unsymmetrical and transformed to a symmetrical matrix.

*           0--symmetrical matrix.      -1--there is no matrix.

*/

DSTATIC int tMatrix2Symmetric( unsigned char **p, int row, int col)

{

    int i, j;

    int flag = 0;

    if( NULL == p)

        return -1;

    for(i = 0; i < row; i++)

    {

        for(j = 0; j < col; j++)

        {

            if( p[i][j] != p[j][i])

            {

                if( p[i][j] > 0)

                    p[j][i] = p[i][j];

                else

                    p[i][j] = p[j][i];

                flag = 1;

            }

        }

    }

    if( flag )

        return 1;

    else

        return 0;

}

DSTATIC void classifyTxPower( sTxPowerClassification *pPowerListHead )

{

    PAutoChannelAndPower    pACPNode = ( PAutoChannelAndPower )headDCA.slList.slHead.next;

 

    while( pACPNode )

    {

        sTxPowerNode *pTxPowerNode = ( sTxPowerNode * )  SWMALLOC( sizeof(sTxPowerNode) );

        if( NULL == pTxPowerNode )

        {

            CRITICAL(("DCA[%s]: Failed to allocate memory!", __FUNCTION__ ));

            return;

        }

 

        memcpy( pTxPowerNode->BSSID, pACPNode->BSSID, sizeof(pACPNode->BSSID) );

        pTxPowerNode->txPowerLvlCurr = pACPNode->txPowerLvlCurr;                /*0--4*/

        slListAdd( &pPowerListHead->txPowerList[pTxPowerNode->txPowerLvlCurr], ( SlListNode* ) pTxPowerNode );

 

        pACPNode = pACPNode->next;

    }

}

/**

* @func: check if last sspp connection has been completed so that we can start next sspp connection.

* @param:

* @return: -1--DCA_STOP_CALC  1--start next(LAST_SSPP_COMPLETION)  0--timeout(LAST_SSPP_TIMEOUNT).

* @NOTE: timeount = 15s.

*/

DSTATIC int checkSSPPCompletion( PSpPeerList pCurrentSpPeer )

{

    PSdpPeer prPtr = NULL;

    int ret = 0, count = 0;

    int prDeviceType = 0;

    unsigned char mac[6] = { 0x00 };

 

    if( NULL == pCurrentSpPeer )

        return DCA_STOP_CALC;

 

    SDP_PROTECT

    prDeviceType = pCurrentSpPeer->prDeviceType;

    memcpy( mac, pCurrentSpPeer->spPrvCfg.id, 6 );

    SDP_RELEASE

 

#if INCLUDE_SONICPOINT_11N

    prPtr = sdpPeerSearch( mac , 0, 0, 0, prDeviceType );

#else

    prPtr = sdpPeerSearch( mac , 0, 0, 0 );

#endif /* INCLUDE_SONICPOINT_11N */

    /*The termination condition should be added to avoid endless loop when the AP is disconnected while its work status is still treated as opeational state.*/

    SDP_PROTECT

    while( ( pCurrentSpPeer->spSession != -1 || SdpStateConnecting == prPtr->prState || SdpStateCommanding == prPtr->prState  ) && ( count < SSPP_TIMEOUT_CNT) )

    {

        SDP_RELEASE

 

        DCA_DEBUG(1,("DCA:[%s] wait last command to be completed!  \n", __FUNCTION__));

        memset( &s_calcTaskPipeCmdCnt, 0x00, sizeof(s_calcTaskPipeCmdCnt) );

        ret = checkCalcTaskPipe( 1 );

 

        if( ret > 0 )

        {

            if( s_calcTaskPipeCmdCnt.m_stopCalcCmdCnt > 0 )

                return DCA_STOP_CALC;

        }

        count++;

 

        SDP_PROTECT

    }

 

    SDP_RELEASE

 

    if( count >= SSPP_TIMEOUT_CNT )

        return LAST_SSPP_TIMEOUNT;

    else

        return LAST_SSPP_COMPLETED;

}

/**

* @func: task for calculating channel.

* @NOTE: 1) we assign channel in default of 20MHZ mode and in 40MHZ mode when designated as 40M-Band by GUI.

*/

DSTATIC void calculateChannelTask()

{

    int ret  = 0, i = 0, coef = 0;

    int row  = 0, col = 0, m_spCnt = 0, dcaNodeIndex = 0;

    long int timeVal = 0;

    int rssiStart = RSSI_THRESHOLD_MINI, rssiEnd = RSSI_THRESHOLD_MAX;

    int prDeviceType = 0, dcaScanAgingTime = 0;

    SpPtGuiMsg      msg  = { 0x00 };

    sTxPowerClassification  powerClassListHead;

 

    srand( (unsigned) time(NULL) );                 /*scatter seed*/

 

    PSpPeerList             pCurrentSpPeer  = NULL; /*points to current sp who executes RF scan operation*/

    SonicPointStatus        *pStatus        = NULL;

    SonicPointScanResultSet *pScanResultSet = NULL;

    SonicPointScanResult    *pScanResult    = NULL;

 

    /*scan RF info one bye one.It needs to update the RSSI info since there may be a new sp added or deleted*/

    PAutoChannelAndPower    pACPNode        = NULL;

    PAutoChannelAndPower    pACPNodeTemp    = NULL;

 

    while(1)

    {

ACP_START:

        memset( &s_calcTaskPipeCmdCnt, 0x00, sizeof(s_calcTaskPipeCmdCnt) );

        ret = checkCalcTaskPipe(300);

        if( ret <= 0)

        {

            /*pipe calcTaskPipe is selected error or timeout*/

            continue;

        }

        else

        {

            /*we only care about the ACP2CALC_START_CALC_CMD at this time. So we clear all other 2 commands statistics*/

            s_calcTaskPipeCmdCnt.m_stopCalcCmdCnt = 0;

            s_calcTaskPipeCmdCnt.m_ssppScanReturn = 0;

            if( s_calcTaskPipeCmdCnt.m_startCalcCmdCnt > 0)

            {

                s_calcTaskPipeCmdCnt.m_startCalcCmdCnt = 0;

                /*we need to check whether the work states of APS is stable.*/

                ret = checkAPOk();                             

                if( NO_AP_CONNECTED == ret || STOP_CALC == ret)

                {

                    /*process before exit if necessary*/

                    continue;

                    /*goto ACP_START;*/

                }

                /*--------prepare memory--------*/

                if( ( SW_ERR_FAIL == setupAdjacentAndRSSIMatrix() ) || ( SW_ERR_FAIL == setupColorScheme() ) )

                {

                    freeColorScheme();

                    freeAdjacentAndRSSIMatrix();

                    continue;

                    /*goto ACP_START;*/

                }

                /*---reset the txpower of all APs---*/

                setPowerOrCh(NULL, SET_POWER, 0);/*it should be put outside the loop of calculation(START_CALC ~ END_CALC).*/

                for( i = 0; i < 5; i++)

                    slListInit(&powerClassListHead.txPowerList[i]);

 

                DCA_DATA_PROTECT( headDCA.semMutex )

 

                DCA_DATA_PROTECT( sACPFlag.semMutex )

                sACPFlag.m_calcTaskRunning = 1;     /* so that sspp will tell the task if the scanning result has been returned.*/

                DCA_DATA_RELEASE( sACPFlag.semMutex )

 

                headDCA.roundOfCalc = 0;

 

                /*update Matrix*/

                updateAdjacentAndRSSIMatrix();

 

                if( SCANMOED_FORCE_ALL_SP_SCAN == headDCA.enum_spScanMode )

                    memsetRSSIMatrix();

                /*---update all sp's channel info.---*/

                if( 0 == updateSPRelatedInfo() )

                {

                    goto CALC_RESTART;

                }

                /* scan 'scanCnt' times and setup rssiMatrix*/

                do

                {

                    DCA_DEBUG(1,("DCA:[%s:%d] calculation has been started!\n", __FUNCTION__, __LINE__))

                    row = 0; col = 0;                   /* the row and col of adjacent matrix*/

 

                    pACPNode = ( PAutoChannelAndPower ) headDCA.slList.slHead.next;

                    while( pACPNode )

                    {

                        if( 0 == pACPNode->nEnableDCA ) /*jump over the node if DCA is disabled.*/

                        {

                            DCA_DEBUG(1,("DCA:[ %s:%d ] SP["MAC_STR_FORMAT"] DCA feature is disabled!\n", __FUNCTION__, __LINE__, MAC_ARRARY(pACPNode->BSSID)))

                            goto NEXT_NODE;

                        }

                        /*--------------------scan (start)----------------------------------------------------------------------------------------------------*/

                        /*find this sp in SpPeerList so as to get the RSSI result*/

                        memcpy( msg.msgMac, pACPNode->BSSID, sizeof(pACPNode->BSSID) );

                        BSSID_TO_ID( msg.msgMac )                /*convert it into id*/

 

                        SDP_PROTECT

                        pCurrentSpPeer = ( PSpPeerList ) searchPeerFromMacSonicPoint( msg.msgMac );

                        if( NULL == pCurrentSpPeer )

                        {

                            /*if we can't find the SP in the peerList, we stop calculating. And if it has been deleted ,it'll restart calculating.*/

                            SDP_RELEASE

                            goto CALC_RESTART;

                        }

                        /* SP has been found in the peerList, so we continue.*/

                        pACPNode->spsWorkState = pCurrentSpPeer->spStatus.spsWorkState;     /*get work status*/

                        prDeviceType           = pCurrentSpPeer->prDeviceType;

 

                        if( SP_STATE_OPERATIONAL == pACPNode->spsWorkState )

                        {

                            DCA_DEBUG(1,("DCA:[%s] Node is operational  \n", __FUNCTION__));

 

                            SDP_RELEASE

                            /* wait for completing the command as it doesn't support that two more consecutive command wite to one AP at the same time.*/

                            ret = checkSSPPCompletion( pCurrentSpPeer );

 

                            if( LAST_SSPP_COMPLETED == ret ) /* we have made it.*/

                            {

                                if( SCANMOED_ALL_SP_SCAN_BASED_ON_TIME == headDCA.enum_spScanMode )

                                {

                                    if( SCANMODE_THIS_SP_SCAN_BASED_ON_TIME == pACPNode->enum_spScanMode )

                                    {

                                        /*scanning or not ,according to the aging time.*/

                                        timeVal = time(NULL);

                                        dcaScanAgingTime = getDCAScanAgingTime()*60;    /*unit:seconds*/

                                        DCA_DEBUG(1, ("DCA: Aging time is %d(min)!\n", dcaScanAgingTime ))

 

                                        if( abs(timeVal - pACPNode->scanTime) > dcaScanAgingTime ) /*aging Time*/

                                        {

                                            /*it need to update RSSI info as aging time is up.*/

                                            msg.msgCmd = SP_PROVISION_SCAN;

                                            write( gPtToGuiMsgFd, (char *)&msg, sizeof(SpPtGuiMsg) );

                                        }

                                        else

                                        {

                                            /*don't need to update RSSI Info and leave it alone.*/

                                            DCA_DEBUG(1,("DCA[ %s:%d ] Node is still in the range of aging time!\n", __FUNCTION__, __LINE__));

                                            goto NEXT_NODE;

                                        }

                                    }

                                    else /*if( SCANMODE_THIS_SP_SCAN_BASED_ON_TIME == pACPNode->enum_spScanMode )*/

                                    {

                                        /*sp state change into operation, we force this sp to scan.*/

                                        msg.msgCmd = SP_PROVISION_SCAN;

                                        write( gPtToGuiMsgFd, (char *)&msg, sizeof(SpPtGuiMsg) );

 

                                        pACPNode->enum_spScanMode = SCANMODE_THIS_SP_SCAN_BASED_ON_TIME;

                                    }

                                }

                                else if( SCANMOED_FORCE_ALL_SP_SCAN == headDCA.enum_spScanMode || SCANMOED_NONE_INITIAL == headDCA.enum_spScanMode )

                                {

                                    /*force all sp to scan*/

                                    /*headDCA.enum_spScanMode will be restored to 'SCANMOED_ALL_SP_SCAN_BASED_ON_TIME' when all scanning have completed. */

                                    msg.msgCmd = SP_PROVISION_SCAN;

                                    write( gPtToGuiMsgFd, (char *)&msg, sizeof(SpPtGuiMsg) );

 

                                    headDCA.enum_spScanMode = SCANMOED_ALL_SP_SCAN_BASED_ON_TIME;

                                    pACPNode->enum_spScanMode = SCANMODE_THIS_SP_SCAN_BASED_ON_TIME;

                                }

                                else

                                {

                                    /*do nothing and reset mode.*/

                                    headDCA.enum_spScanMode = SCANMOED_ALL_SP_SCAN_BASED_ON_TIME;

                                    goto ACP_START;

                                }

                            }

                            else if( LAST_SSPP_TIMEOUNT == ret )

                            {

                                /*failed to wait sspp return.*/

                                DCA_DEBUG(1,("DCA[ %s:%d ] Node is not operational!\n", __FUNCTION__, __LINE__));

                                memset( &pACPNode->s_rfa, 0x00, sizeof(pACPNode->s_rfa) );

                                goto NEXT_NODE;

                            }

                            else /* DCA_STOP_CALC == ret */

                            {

                                goto CALC_RESTART;

                            }

                        }/*end_operational*/

                        else    /*jump over this node and keep it isolate from ohter nodes as it's not controllable.*/

                        {

                            SDP_RELEASE

 

                            DCA_DEBUG(1,("DCA[ %s:%d ] Node is not operational!\n", __FUNCTION__, __LINE__));

                            memset( &pACPNode->s_rfa, 0x00, sizeof(pACPNode->s_rfa) );

                            goto NEXT_NODE;

                        }

                        /*----------------------check pipes--(start)--------------------------------------------------------------------------------*/

                        /*  (1)check if new Aps join.if yes, return.(2)check if the scanning result has been received.if yes, continue to the next one.*/

                        pACPNode->m_scanReturn = SSPP_RETURN_INVALID;

                        memset( &s_calcTaskPipeCmdCnt, 0x00, sizeof(s_calcTaskPipeCmdCnt) );

                        ret = checkCalcTaskPipe(50);

 

                        if( ret <= 0)

                        {

                            /*The AP doesn't return the scan result due to some unpredictable cases, such as being cut off power, rebooting or other things.

                              Now we just jump over it and keep it isolate as well.*/

                            DCA_DEBUG(1,("DCA[ %s : %d ] It's timeout to wait the scanning result to return!\n", __FUNCTION__, __LINE__ ))

                            memset( &pACPNode->s_rfa, 0x00, sizeof(pACPNode->s_rfa) );

                            pACPNode->m_scanReturn = SSPP_RETURN_TIMEOUNT;

                            goto NEXT_NODE;

                        }

                        /* [check command] we only care about  ACP2CALC_STOP_CALC_CMD and SSPPSCAN_RETURN at this time. so we clear all other commands statistics.*/

                        s_calcTaskPipeCmdCnt.m_startCalcCmdCnt  = 0;

                        if( s_calcTaskPipeCmdCnt.m_stopCalcCmdCnt > 0 )

                        {

                            /*process before exit if necessary*/

                            goto CALC_RESTART;

                        }

                        else if( s_calcTaskPipeCmdCnt.m_ssppScanReturn > 0 )

                        {

                            /*the scan result has been return*/

                            pACPNode->m_scanReturn = SSPP_RETURN_OK;

                            pACPNode->scanTime = time(NULL);

                            DCA_DEBUG(1,("DCA[ %s : %d ] The scan result has returned!\n", __FUNCTION__, __LINE__ ))

 

                            SDP_PROTECT                 //As the peerList will be destined to be identical to pACPNodeList.

                            if( NULL == pCurrentSpPeer)

                            {

                                /*this peer -has been deleted from SpPeerList ,but sync with the data structure here.*/

                                /*Actually, it comes to this branch only when the pipe msg has been received.*/

                            }

                            else

                            {

                                /*get if the channel of AP is designated by user.Just for standard 20MHZ. completed in updateSPInfo()*/

                                DCA_DEBUG(1,("Find this sp in SpPeerList so as to get the RSSI result:"))

                                DCA_DEBUG(1,("Scan id= "MAC_STR_FORMAT"\n", MAC_ARRARY(pCurrentSpPeer->spPrvCfg.id) ))

 

                                pStatus         = ( SonicPointStatus* ) &( pCurrentSpPeer->spStatus );

                                pScanResultSet  = ( SonicPointScanResultSet* ) &( pStatus->spsScanResSet[0] );//A_MODE

                                DCA_DEBUG(1,( "ScanACount= %d \n", pScanResultSet->scanGCount))

                                pScanResultSet  = ( SonicPointScanResultSet* ) &( pStatus->spsScanResSet[1] );//G_MODE

                                DCA_DEBUG(1,( "ScanGCount= %d \n", pScanResultSet->scanGCount))

                                /*----fill the adjacent matrix in the format as follows:

                                (sp1->mac1)|    RSSI1(0)    RSSI1_2    ...      | <-- find how many APs in the scan results

                                                                                      and mark the uncontrollable AP to prepare for RFA.

                                (sp2->mac2)|    RSSI2_1     RSSI2(0)   ...      | <-- ...

                                 ...            ...         ...        ...      | <-- ..

                                */

                                /*traverse the result of sp*/

                                col = 0;

                                dcaNodeIndex = 0;

                                memset( headDCA.findFlag, TAKEN_INTO_RFA, sizeof(headDCA.findFlag) );

                                pACPNodeTemp = ( PAutoChannelAndPower ) headDCA.slList.slHead.next;

                                while( pACPNodeTemp )

                                {

                                    DCA_DEBUG(1,("Find BSSID= "MAC_STR_FORMAT"\n", MAC_ARRARY(pACPNodeTemp->BSSID) ))

                                    DCA_DEBUG(1,( "-------------------------------------------------------------------------------- \n"))

                                    /*G_MODE.*/

                                    for( m_spCnt = 0; m_spCnt < pScanResultSet->scanGCount; m_spCnt++ )     //SONICPOINT_SCANRESULT_MAX

                                    {

                                        pScanResult = (SonicPointScanResult *)&(pScanResultSet->scanResult[m_spCnt]);

 

                                        DCA_DEBUG(1,("m_spCnt = %-2d, pScanResult->BSSID = "MAC_STR_FORMAT", pScanResult->channel = %ld, pScanResult->RSSI = %ld\n", \

                                                    m_spCnt, MAC_ARRARY(pScanResult->BSSID), pScanResult->channel, pScanResult->RSSI ))

                                        /*for Rfa (to be extended to 2 channel)*/

                                        /* The rssi is accurate even if the channel has been changed during the scannig process.*/

                                        headDCA.s_rfEvaluate.m_scanResultRSSI[m_spCnt][0]   = pScanResult->RSSI;

                                        headDCA.s_rfEvaluate.chanFreq[m_spCnt][0]           = pScanResult->channel;

                                        DCA_DEBUG(1,("scanResultRSSI[ %d ] = %d, chFreq[ %d ] = %d\n", m_spCnt,pScanResult->RSSI,m_spCnt,pScanResult->channel))

#if 0

/* to do--NDR*/

                                        headDCA.s_rfEvaluate.m_scanResultRSSI[m_spCnt][1]   = pScanResult->RSSI;

                                        headDCA.s_rfEvaluate.chanFreq[m_spCnt][1]           = pScanResult->channel;

#endif

                                        /*compare if there is a RSSI related with pACPNodeTemp->BSSID*/

                                        if( 0 == memcmp( pACPNodeTemp->BSSID, pScanResult->BSSID, 5 ) ) /*eliminate VAP from scanning result*/

                                        {

                                            if( pACPNodeTemp->BSSID[5] == pScanResult->BSSID[5] )

                                            {

                                                /*find and read out RSSI fill in the corresponding position in adjacent matrix*/

                                                DCA_DEBUG(1,("find in the scanning result!\n", headDCA.m_rssiCurThreshold))

                                                /*Actually, we can scan more times to do error analysis to exclude some uncredible RSSI under unstable state, but it'll have a great effection on the performance of wireless.So default once here.*/

                                                if( 0 == headDCA.roundOfCalc )

                                                {

                                                    /*round 0*/

                                                    headDCA.pRSSIMatrix[row][col] = pScanResult->RSSI;

                                                }

                                                else if( headDCA.roundOfCalc )

                                                {

                                                    /* round 1,2,3...*/

                                                    /*actually, there is error theory which can be used to filter uncredible data point.*/

                                                    if( abs( pScanResult->RSSI - headDCA.pRSSIMatrix[row][col] )  < RSSI_ACCEPTABLE_INTERVAL ) /*the acceptable jumping interval.*/

                                                        headDCA.pRSSIMatrix[row][col] =  ( pScanResult->RSSI + headDCA.pAdjacentMatrix[row][col] ) / 2;

                                                    else

                                                    {

                                                        headDCA.pRSSIMatrix[row][col] = headDCA.pRSSIMatrix[row][col] > pScanResult->RSSI ? headDCA.pRSSIMatrix[row][col]: pScanResult->RSSI;

                                                    }

                                                }

 

                                                if( headDCA.pAPFixedCh[dcaNodeIndex][0] == AUTO_CHANNEL_SP )    /*managed APS with fixed channels should be taken into account when do rfa.*/

                                                {

                                                    headDCA.findFlag[m_spCnt] = NOT_TAKEN_INTO_RFA;

                                                }

                                                else

                                                    headDCA.findFlag[m_spCnt] = TAKEN_INTO_RFA;

                                            }

                                            /*eliminate vap*/

                                            if( pACPNodeTemp->vapBssidList.resolved )

                                            {

                                                if( pScanResult->BSSID[5] - pACPNodeTemp->BSSID[5] <= MAX_NUM_VAP-1 )

                                                {

                                                    headDCA.findFlag[m_spCnt] = NOT_TAKEN_INTO_RFA;

                                                }

                                            }

                                            else

                                            {

                                                break;

                                            }

                                        }

                                    }

                                    pACPNodeTemp = pACPNodeTemp->next;

                                    col++;

                                    dcaNodeIndex++;

                                }/*end_while( pACPNodeTemp )_node traversal*/

                            }/*end_else_pCurrentSpPeer!=NULL*/

                            SDP_RELEASE

                        }/*end_scan_process*/

                        /*under other cases,we just ignore this node since no scan result and no stop command has been received.*/

                        /*----------------------check pipes--(end)--------------------------------------------------------------------------------*/

                        /*-----------------------------------RFA part(start)---G_MODE--------------------------------------------------------------------*/

                        /* RFA on channels (we only take 1,6 and 11 into account at present)according to the RSSI of unmanaged APs so that we get suboptimum assignment

                            when there is no ideal result after calculation.*/

                        /*firstly, we check if it's necessary to do rfa*/

                        headDCA.isNeedDoRfa = 0;

                        for( m_spCnt = 0; m_spCnt < pScanResultSet->scanGCount; m_spCnt++ )

                        {

                            DCA_DEBUG(1,("DCA[ %s:%d ] headDCA.findFlag[%d] = %d\n", __FUNCTION__, __LINE__, m_spCnt, headDCA.findFlag[m_spCnt]));

                            if( TAKEN_INTO_RFA == headDCA.findFlag[m_spCnt] ) /* there are managed APs with fixed channel or unmanaged APs around current AP.*/

                            {

                                headDCA.isNeedDoRfa = 1;

                                DCA_DEBUG(1,("DCA[ %s:%d ] it needs to do rfa!\n", __FUNCTION__, __LINE__));

                                break;

                            }

                        }

                        if( 1 == headDCA.isNeedDoRfa && 0 == pACPNode->s_radioInfo[1].ch[0] ) //it makes sense only when channel has been set to 'auto'.

                        {

                            /*rfa based on unmanaged APs or managed APs with fixed channel*/

                            for( i = 1; i <= 11; i++)  //2.4G--channel===i

                            {

                                if( 0 == headDCA.roundOfCalc )

                                {

                                    pACPNode->s_rfa.rfaCh[i] = i;     /*1--11*/

                                    pACPNode->s_rfa.rfScore[i] = 0;

                                }

                                for( m_spCnt = 0; m_spCnt < pScanResultSet->scanGCount; m_spCnt++ )

                                {

                                    DCA_DEBUG(1,("DCA[ %s:%d ] headDCA.findFlag[%d] = %d\n", __FUNCTION__, __LINE__, m_spCnt, headDCA.findFlag[m_spCnt]))

                                    if( TAKEN_INTO_RFA == headDCA.findFlag[m_spCnt] )

                                    {

                                        DCA_DEBUG(1,("[Take into RFA] m_spCnt = %d \n", m_spCnt))

                                        /*unmanaged APs or Aps with fixed channels*/

                                        if(abs(headDCA.s_rfEvaluate.chanFreq[m_spCnt][0]-frequency24G[pACPNode->s_rfa.rfaCh[i]]) <= 20)

                                        {

                                            /*take channels related to channel i into consideration.*/

                                            coef = rfaGetInterferenceCoefFreq( frequency24G[ pACPNode->s_rfa.rfaCh[i] ], headDCA.s_rfEvaluate.chanFreq[m_spCnt][0] );

                                            pACPNode->s_rfa.rfScore[ i ] = rfaOverflowCheckAdd( pACPNode->s_rfa.rfScore[ i ],\

                                                                           headDCA.s_rfEvaluate.m_scanResultRSSI[ m_spCnt ][0]/coef );

                                            DCA_DEBUG(1,("coef = %d\n", coef))

                                            DCA_DEBUG(1,("rfaScore[%d] = %d\n", i,pACPNode->s_rfa.rfScore[ i ]))

                                        }

                                    }

                                    else

                                        DCA_DEBUG(1,("[not take into RFA] m_spCnt = %d \n", m_spCnt))

                                        DCA_DEBUG(1,("------------------------------------------\n"))

                                }

                                DCA_DEBUG(2,("DCA[%s:%d] pACPNode->rfScore[%d] = %d\n", __FUNCTION__, __LINE__, pACPNode->s_rfa.rfaCh[i], pACPNode->s_rfa.rfScore[i]));

                            }

                        }

                        else

                        {

                            for( i = 1; i <= NUM_USING_CHANNEL; i++ )

                            {

                                pACPNode->s_rfa.rfScore[i] = 0;

                                pACPNode->s_rfa.rfaCh[i] = i;

                            }

                        }

                        /*-----------------------------------RFA part(end)---------------------------------------------------------------------------*/

                        if( (scanCnt-1) == headDCA.roundOfCalc )/*last scan*/

                        {

                            /*-----------------------------------sort optional channel in order (start)--------------------------------------------------*/

                            if( pACPNode->s_radioInfo[1].ch[0] != 0 )   /*fixed channels*/

                            {

                                /*fixed channel and on optional channel*/

                                memset( pACPNode->optionalCh, 0x00, sizeof( pACPNode->optionalCh )/sizeof( unsigned char ) );

                            }

                            else

                            {

                                /*decide the order of channels to be asssigned based on RFA.*/

                                DCA_DEBUG(1,("DCA[ %s:%d ] it needs to decideOptionalCh!\n", __FUNCTION__, __LINE__));

                                decideOptionalCh( pACPNode ,NULL);

                            }

                        }

                        /*-----------------------------------sort optional channel in order (end)--------------------------------------------------*/

NEXT_NODE:

                        pACPNode = pACPNode->next;

                        row++;  /*do statistics of next ap*/

                    }/*end_while( pACPNode )_traversal*/

 

                    headDCA.roundOfCalc++;

                }while( headDCA.roundOfCalc < scanCnt );  /*end_while( headDCA.roundOfCalc <= 1 )_scan 2 times*/

 

                DCA_DATA_PROTECT( sACPFlag.semMutex )

                sACPFlag.m_calcTaskRunning = 0;     /* so that sspp will tell the task if the scanning result has been returned.*/

                DCA_DATA_RELEASE( sACPFlag.semMutex )

                /*------------------------------------------calculating (start)----------------------------------------------------------------------*/

                /*set rssiThreshold */

                rssiStart = RSSI_THRESHOLD_MINI;

                rssiEnd   = RSSI_THRESHOLD_MAX;

                headDCA.m_rssiCurThreshold  = rssiStart;             /*we check if there is a solution under lowest rssi threshold.*/

                while(1)

                {

                    memsetAdjacentMatrix();

                    for( row = 0; row < headDCA.slList.count; row++ )    /*setup Adjacent Matrix according to RSSI Matrix.*/

                    {

                        for( col = 0; col < headDCA.slList.count; col++ )

                        {

                            headDCA.pAdjacentMatrix[row][col] = headDCA.pRSSIMatrix[row][col] > headDCA.m_rssiCurThreshold ? 1 : 0;

                        }

                    }

                    tMatrix2Symmetric( headDCA.pAdjacentMatrix, row, col);

                    memsetColorScheme();    /*clear pVertexColor and pAssignmnet*/

//                  giveIdealAssign(0);     /*vertex = 0*/

                    giveOneAssign( (PAutoChannelAndPower)headDCA.slList.slHead.next, 0);

                    /*1.check if there is a solution under ideal condition.if not, we increase the threshold of rssi until it exceed

                        the maximum threshold of rssi so that we can get a ideal solution. rssi_threshold += delta.

                      2.if no solution has been calculated, we'll lower the tx power to try again. */

                    if ( 0 == headDCA.s_colorScheme.schemeCnt )

                    {

                        /*no solution, but we need to check if  rssiStart is so close that no other threshold we can try, eg: rssiStart=53, rssiEnd=54(ok),threshold=53,the program enter into dead loop!*/

                        DCA_DEBUG(1,("DCA: no solution!\n"))

 

                        if( RSSI_THRESHOLD_MINI == headDCA.m_rssiCurThreshold ) /*mini -- no solution*/

                            headDCA.m_rssiCurThreshold = RSSI_THRESHOLD_MAX;    /*try max*/

                        else if( headDCA.m_rssiCurThreshold  >= RSSI_THRESHOLD_MAX )/* max -- no solution*/

                        {

                            rssiStart = headDCA.m_rssiCurThreshold;

                            headDCA.m_rssiCurThreshold += RSSI_THRESHOLD_DELTA; /*to close in the best result quickly*/

                        }

                        else if( headDCA.m_rssiCurThreshold > RSSI_THRESHOLD_MINI && headDCA.m_rssiCurThreshold < RSSI_THRESHOLD_MAX )

                        {

                            if( rssiStart == headDCA.m_rssiCurThreshold )

                            {

                                /*we must move forward here, or it'll get lost in dead loop.

                                 Eg:rssiStart=53(no ok) rssiEnd=54(ok), threshold = (53+54)/2 =53. if we don't move to 54, the threshold will be 53 all the time.

                                 That's why we move forward here.*/

                                headDCA.m_rssiCurThreshold = rssiEnd;       //to stop trying to find a ideal threshold as rssiStart is so close to rssiEnd.

                            }

                            else

                            {

                                rssiStart = headDCA.m_rssiCurThreshold;

                                headDCA.m_rssiCurThreshold = ( rssiStart + rssiEnd ) / 2;

                            }

                        }

                        headDCA.roundOfCalc++;

                        DCA_DEBUG(1,("rssiStart = %d, rssiEnd = %d\n", rssiStart, rssiEnd))

                    }

                    else

                    {

                        DCA_DEBUG(1,("DCA: solution is ok!\n")  )

                        /* there is a solution.*/

                        if( RSSI_THRESHOLD_MINI == headDCA.m_rssiCurThreshold ) /*mini -- ok, that's ok.*/

                        {

                            break;

                        }

                        else if( headDCA.m_rssiCurThreshold > RSSI_THRESHOLD_MAX )

                        {

                            /*Aps have been deployed so intensive that we may need to adjust txPower in some of them.*/

                            /*adjust tx power*/

                            /*set adjust power Flag*/

//                             break;

                        }

                        else if( headDCA.m_rssiCurThreshold > RSSI_THRESHOLD_MINI && headDCA.m_rssiCurThreshold <= RSSI_THRESHOLD_MAX )

                        {

                        }

 

                        if( ( rssiEnd - rssiStart ) < RSSI_STOP_INTERVAL )

                            break;

                        rssiEnd = headDCA.m_rssiCurThreshold;

                        headDCA.m_rssiCurThreshold = ( rssiStart + rssiEnd ) / 2;

                        DCA_DEBUG(1,("rssiStart = %d, rssiEnd = %d\n", rssiStart, rssiEnd))

                    }

                }/*end_while(1)_calculation*/

                /*--------------------------calculation(end)--------------------------------------------------------------------*/

                //set the channel into Ap.

                setPowerOrCh(NULL, SET_CHANNEL, 0);

CALC_RESTART:

                DCA_DATA_PROTECT( sACPFlag.semMutex )

                sACPFlag.m_calcTaskRunning = 0;     /* so that sspp will tell the task if the scanning result has been returned.*/

                DCA_DATA_RELEASE( sACPFlag.semMutex )

 

                DCA_DATA_RELEASE( headDCA.semMutex )

 

            }//end_if(start to calculate)

        }//end_if_check_pipe_1

    }//end_while(1)

 

    /*error*/

    freeColorScheme();

    freeAdjacentAndRSSIMatrix();

    return;

}

/**

 *@func: auto channel and power task

*/

DSTATIC int autoChannelAndPowerTask()

{

    fd_set          readFds;

    struct timeval  timeOut;

    SpPtGuiMsg      msg;

    int n = 0, ret = 0, msgLen = 0;

    int m_stopCalcFlag = ACP2CALC_STOP_CALC_CMD;

    int m_startCalcFlag = ACP2CALC_START_CALC_CMD;

    int i = 0;

 

    spInfoInit();   /* initialize the linked-list of sp since the addPeerByGUISonicPoint can't write info to this task.*/

 

    if( ( taskSpawn( "tCalcCh", 201, 0, 40000, (FUNCPTR)calculateChannelTask, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 ) ) == ERROR )

    {

        CRITICAL(("DCA: Fail to spawn task for calculating channels!\n"));

        return SW_ERR_FAIL;

    }

    while(1)

    {

        FD_ZERO( &readFds );

        FD_SET( gDynamicChannelFd, &readFds );

        timeOut.tv_sec  = getDCALatencyTimeForNewSP();

        timeOut.tv_usec = 0;

        n               = gDynamicChannelFd;

        ret             = select( n+1, &readFds, NULL, NULL, &timeOut );

 

        if ( ret < 0 )

        {

            DCA_DEBUG( 1, ("DCA[%s : %d]: select error!\n", __FUNCTION__, __LINE__) )

            continue;

        }

        if ( ret == 0 )

        {

            /*timeOut*/

            if( 1 == m_justBoot)

            {

                DCA_DEBUG( 1, ("DCA: The system has just started and needed to assign channels to all SP!\n") )

                m_justBoot = 0;

                m_channelAssignFlag = 1;

                headDCA.enum_spScanMode = SCANMOED_FORCE_ALL_SP_SCAN;

            }

            else if( 1 == m_channelAssignFlag)

            {

                m_channelAssignFlag = 0;  /*only once*/

 

                if( ( n = write( gCalcTaskPipeFd, (char *)&m_startCalcFlag, sizeof(int) ) ) != sizeof(int) )

                {

                    CRITICAL(("DCA: Failed to write pipe[ gCalcTaskPipeFd ]!"))

                    return SW_ERR_FAIL;

                }

            }

            continue;

        }

 

        /* triggered by SDP */

        if( FD_ISSET( gDynamicChannelFd, &readFds ) )

        {

            /*stop the task for calculating channel at first since calculation doesn't make sense when spPeerList has been changed*/

            if( (n = write( gCalcTaskPipeFd, (char *)&m_stopCalcFlag, sizeof(int))) != sizeof(int) )

            {

                CRITICAL(("DCA: Failed to write pipe[gCalcTaskPipeFd]!"))

                return SW_ERR_FAIL;

            }

            taskDelay(10);

            /*read new SPS to update spInfo*/

//          n = ioctl(gDynamicChannelFd, FIONMSGS , 0 );

            msgLen = read( gDynamicChannelFd, (char *)&msg, sizeof(SpPtGuiMsg) * ACPTASK_PIPE_MAX_MSGS );

            for( i = 0; i < msgLen/sizeof(SpPtGuiMsg); i++ )

                updateSPInfo( &msg );  /*if calcTask hasn't exited, it will block here.*/

        }

    }

    return 0;

}

/**

 *@ initialize dynamic channel assignment module.

*/

int autoChannelAndPowerInit()

{

    /*get schedule handle*/

    gDCASchedParam.dcaSchedOldHandle = gDCASchedParam.dcaSchedHandle;   /*save temperarily*/

    memset( &headDCA, 0x00, sizeof(AutoChannelAndPowerHeader) );    /*initze header(important)*/

 

    if( ( headDCA.semMutex = swSemCreate("headDCA.semMutex", TRUE, FALSE, NULL)) == NULL )

    {

        CRITICAL(("DCA:Failure to create semaphore [headDCA.semMutex]", __FUNCTION__, __LINE__ ));

        return SW_ERR_FAIL;

    }

 

    if( ( sACPFlag.semMutex = swSemCreate("sACPFlag.semMutex", TRUE, FALSE, NULL)) == NULL )

    {

        CRITICAL(("DCA:Failure to create semaphore [sACPFlag.semMutex]"));

        return SW_ERR_FAIL;

    }

    sACPFlag.m_calcTaskRunning = 0;

    sACPFlag.m_acpTaskRunning  = 0;

 

    headDCA.enum_spScanMode = SCANMOED_FORCE_ALL_SP_SCAN;   /*force all sp to scan at the first time.*/

    /* auto channel and auto power */

    if( taskSpawn("tACP", 200, 0, 10000, (FUNCPTR)autoChannelAndPowerTask,0,  0, 0, 0, 0, 0, 0, 0, 0, 0) == ERROR )

    {

        CRITICAL(("DCA:Fail to spawn task for auto dynamic channel assignment!"));

        return SW_ERR_FAIL;

    }

    return 0;

}

#if DEBUGALL

/*if debug is enabled, we shouldn't free the memory of matrix and the result of calculation.*/

void showMatrix()

{

    int i = 0, j = 0;

    DCA_DATA_PROTECT( headDCA.semMutex)

    printf("---------------------[Adjacent Matrix-start]----------------------\n");

    if( NULL != headDCA.pAdjacentMatrix)

    {

        for(i = 0; i < headDCA.m_matrixLen; i++)

        {

            if( NULL != headDCA.pAdjacentMatrix[i] )

            {

                printf("\n|");

                for(j = 0; j < headDCA.m_matrixLen; j++)

                {

                    printf("%d ", headDCA.pAdjacentMatrix[i][j]);

                    if( j != ( headDCA.m_matrixLen - 1) )    //not the last node of a row.

                        printf("\t");

                }

                printf("|");

            }

        }

    }

    else

        printf( "headDCA.pAdjacentMatrix = NULL ...\n");

    printf("\n");

    DCA_DATA_RELEASE( headDCA.semMutex)

    return;

}

void showRSSIMatrix()

{

    int i = 0, j = 0;

    DCA_DATA_PROTECT( headDCA.semMutex)

    printf("---------------------[RSSI Matrix-start]----------------------\n");

    if( NULL != headDCA.pRSSIMatrix)

    {

        for(i = 0; i < headDCA.m_matrixLen; i++)

        {

            if( NULL != headDCA.pAdjacentMatrix[i] )

            {

                printf("\n|");

                for(j = 0; j < headDCA.m_matrixLen; j++)

                {

                    printf("%d", headDCA.pRSSIMatrix[i][j]);

                    if( j != ( headDCA.m_matrixLen - 1) )    //not the last node of a row.

                        printf("\t");

                }

                printf("|");

            }

        }

    }

    else

        printf( "headDCA.pRSSIMatrix = NULL ...\n");

    printf("\n");

    DCA_DATA_RELEASE( headDCA.semMutex)

    return;

}

void showAP()

{

    struct tm * timeinfo;

    int i = 0, count = 0;

    const char *pChModeBand[8]={

        "CH_24G_20M_AUTO ", "CH_24G_20M_FIXED", "CH_24G_40M_AUTO ", "CH_24G_40M_FIXED",

        "CH_5G_20M_AUTO  ", "CH_5G_20M_FIXED ", "CH_5G_40M_AUTO  ", "CH_5G_40M_FIXED",

    };

    const char *pSpScanMode[4]={

        "SCANMOED_NONE_INITIAL ", "SCANMOED_FORCE_ALL_SP_SCAN",

        "SCANMOED_ALL_SP_SCAN_BASED_ON_TIME ", "SCANMOED_INVALID",

    };

    DCA_DATA_PROTECT( headDCA.semMutex)

 

    printf("####################################[show dcaTask info as follows]##############################\n");

    printf("---------------------------[ACP Task]----------------------------\n");

    printf("m_channelAssignFlag     =%d\n", m_channelAssignFlag);

    if( m_channelAssignFlag > 0)

        printf("Note:Calc Task will be restarted since it's need to assign channel based on new topology!\n");

    else

        printf("Note:Calc Task is sleeping!\n");

    printf("---------------------------[Calc Task]----------------------------\n");

    printf("####################################[show SP info as follows]###################################\n");

    printf("---------------------------[Head info]----------------------------\n");

    printf("headDCA.m_matrixLen         = %d\n", headDCA.m_matrixLen );

    printf("headDCA.slList.count        = %d\n", headDCA.slList.count);

    printf("headDCA.oldListCount        = %d\n", headDCA.m_oldListCount);

    printf("headDCA.m_rssiCurThreshold  = %d\n", headDCA.m_rssiCurThreshold );

    printf("headDCA.roundOfCalc         = %d\n ", headDCA.roundOfCalc );

    printf("headDCA.isNeedDoRfa         = %d\n ", headDCA.isNeedDoRfa );

    printf("headDCA.enum_spScanMode     = %s\n ", pSpScanMode[headDCA.enum_spScanMode] );

    printf("headDCA.s_InfoUpdateMatrix:\n");

    for( i = 0; i < headDCA.s_InfoUpdateMatrix.delCount; i++)

    {

        printf("%d  \n", headDCA.s_InfoUpdateMatrix.delIndexArray[i]);

        if( i % 16 == 0 )

            printf("\n");

    }

    printf("headDCA.s_InfoUpdateMatrix.delCount = %d\n", headDCA.s_InfoUpdateMatrix.delCount );

    printf("headDCA.s_InfoUpdateMatrix.addCount = %d\n", headDCA.s_InfoUpdateMatrix.addCount );

    printf("headDCA.s_InfoUpdateMatrix.validIndex = %d\n", headDCA.s_InfoUpdateMatrix.validIndex );

 

    showRSSIMatrix();

    showMatrix();

    /*for Rfa*/

    for( i = 0; i < SONICPOINT_SCANRESULT_MAX && headDCA.s_rfEvaluate.m_scanResultRSSI[i][0];  i++)

    {

        if( 0 == i)

                printf("---------------------------[rfa]----------------------------\n");

        printf("headDCA.s_rfEvaluate.m_scanResultRSSI[ %d ] = %d\n", i, headDCA.s_rfEvaluate.m_scanResultRSSI[i][0]);

        printf("headDCA.s_rfEvaluate.chanFreq[ %d ]         = %d\n", i, headDCA.s_rfEvaluate.chanFreq[i][0]);

    }

 

    if( headDCA.s_colorScheme.schemeCnt > 0)

    {

        printf("---------------------------[scheme]----------------------------\n");

        printf("headDCA.s_colorScheme.schemeCnt             = %d\n", headDCA.s_colorScheme.schemeCnt);

        printf("The result of ynamic channel assignment are as follows:\n");

        for( i = 0; i < headDCA.m_matrixLen; i++)

        {

            printf("head.s_colorScheme.pVertexColor[ %2d ] = %d\n", i, headDCA.s_colorScheme.pVertexColor[i] );

        }

    }

    else

    {

        printf("(V_V!)--No sheme is calculated!\n");

    }

    printf("---------------------------[Node info]----------------------------\n");

    if( 0 == headDCA.slList.count )

    {

        printf("(V_V!)--No Ap is connected!\n");

    }

    else

    {

        if( headDCA.s_colorScheme.schemeCnt > 0 )

        {

            PAutoChannelAndPower pACPNode = (PAutoChannelAndPower) (headDCA.slList.slHead.next);

            while( pACPNode )

            {

                printf("[%2d]."MAC_STR_FORMAT"\n", count++, MAC_ARRARY( pACPNode->BSSID ) );

 

                timeinfo = localtime ( &(pACPNode->scanTime) );

                printf("      Last Scan time is %s\n", asctime(timeinfo));

                if( pACPNode->spsWorkState == SP_STATE_OPERATIONAL && pACPNode->nEnableDCA )

                {

                    for( i = 1; i <= 11; i++)

                    {

                        printf("     rfScore[ %d ] = %-d\n", pACPNode->s_rfa.rfaCh[i], pACPNode->s_rfa.rfScore[i] );

                    }

 

                    for( i = 0 ; i < OPTIONAL_CHANNEL_NUM; i++)

                    {

                        printf("     optionalCh[ %d ] = %d\n", i, pACPNode->optionalCh[i]);

                    }

                    printf("     s_radioInfo[1].enum_modeAndBand = %s\n", pChModeBand[pACPNode->s_radioInfo[1].enum_modeAndBand] );

                    printf("     primary channel                 = %d\n", pACPNode->s_radioInfo[1].ch[0] );

                    printf("     secondary channel               = %d\n", pACPNode->s_radioInfo[1].ch[1] );

                }

                else

                {

                    printf("(V_V!)--this AP isn't operational, please check if it's connected correctly!\n");

                }

                printf("     nEnableDCA                      = %s\n", pACPNode->nEnableDCA ? "Enable":"Disable" );

                pACPNode = pACPNode->next;

            }

        }

        else

        {

            printf("(V_V!)--no scheme has been calculated!\n");

            printf("(^_^!)--connected AP's info as follows:\n");

            PAutoChannelAndPower pACPNode = (PAutoChannelAndPower) (headDCA.slList.slHead.next);

            count = 0;

            while( pACPNode )

            {

                if( pACPNode->spsWorkState == SP_STATE_OPERATIONAL )

                {

                    printf("[%2d]."MAC_STR_FORMAT"\n", count++, MAC_ARRARY(pACPNode->BSSID ) );

                    printf("     s_radioInfo[1].enum_modeAndBand = %s\n", pChModeBand[pACPNode->s_radioInfo[1].enum_modeAndBand] );

                    printf("     primary channel                 = %d\n", pACPNode->s_radioInfo[1].ch[0] );

                    printf("     secondary channel               = %d\n", pACPNode->s_radioInfo[1].ch[1] );

                }

                else

                {

                    printf("(V_V!)--this AP isn't operational, please check if it's connected correctly!\n");

                }

                printf("      nEnableDCA = %s\n", pACPNode->nEnableDCA ? "Enable":"Disable" );

                pACPNode = pACPNode->next;

            }

        }

    }

    DCA_DATA_RELEASE( headDCA.semMutex)

    return;

}

 

void showRfScore()

{

    SpRfaIndexNode *curNode = NULL;

 

    if (NULL == g_spRfaTree.spList )

        return;

 

    if (g_spRfaTree.spList)

        rfaFreeList();

 

    if (rfaInitIndexList(&g_spRfaTree.spList) != SW_ERR_FAIL)

        g_spRfaTree.tableSize = rfaBuildSpList(g_spRfaTree.spList);

 

    curNode = (SpRfaIndexNode *)lstFirst(g_spRfaTree.spList->list);

    while(curNode != NULL)

    {

        if(!curNode->isChannelHead){

            curNode = (SpRfaIndexNode*)lstNext((NODE *)curNode);

            DCA_DEBUG(1,("not channel Head...\n"));

            continue;

        }

        printf("curNode->rfScore[0] = %d,", curNode->rfScore[0]);

        printf("curNode->rfScore[1] = %d\n", curNode->rfScore[1]);

        printf("curNode->chNum[0]   = %d,", curNode->chNum[0]);

        printf("curNode->chNum[1]   = %d\n", curNode->chNum[1]);

        curNode = (SpRfaIndexNode*)lstNext((NODE *)curNode);       

    }

    return;

}

 

void scanAll()

{

  SpPtGuiMsg      msg;

  msg.msgCmd  = SP_PROVISION_SCAN;

 

  PSpPeerList pCurrent                      = NULL;

  SonicPointStatus *pStatus                 = NULL;

  SonicPointScanResultSet *pScanResultSet   = NULL;

  SonicPointScanResult *pScanResult         = NULL;

 

  SDP_PROTECT;

  pCurrent = (PSpPeerList)(gSonicPointSlList.slHead.next);

  while(pCurrent)

  {

    memcpy( msg.msgMac, pCurrent->spPrvCfg.id, 6 );

    printf("DCA[ %s:%d ] scan SP-%x:%x:%x:%x:%x:%x\n", __FUNCTION__, __LINE__, msg.msgMac[0], msg.msgMac[1],msg.msgMac[2],msg.msgMac[3],msg.msgMac[4],msg.msgMac[5]);

   

    write(gPtToGuiMsgFd, (char *)&msg, sizeof(SpPtGuiMsg));

    pCurrent = pCurrent->spNext;

    SDP_sleep(3);

  }

  SDP_RELEASE;

 

  SDP_sleep(5);

  printf("DCA[ %s:%d ] print the result of scan as follows:\n", __FUNCTION__, __LINE__);

  pCurrent = (PSpPeerList)(gSonicPointSlList.slHead.next);

 

  int i = 0;

  int j = 0;

  int m_spCnt = 0;

  while(pCurrent)

  {

        //peerPtr->spStatus.spsScanResSet[0].scanACount

#if 1

        /** A MODE*/

        printf("scan in A mode\n");

        pStatus = (SonicPointStatus*)&(pCurrent->spStatus);

        pScanResultSet = (SonicPointScanResultSet*)&(pStatus->spsScanResSet[0]);

        for( m_spCnt = 0; m_spCnt < pScanResultSet->scanACount; m_spCnt++)

        {

            pScanResult = (SonicPointScanResult *)&(pScanResultSet->scanResult[m_spCnt]);

            printf("----------------------------------------------------\n");

            printf("[SP %d] mac "MAC_STR_FORMAT"\n", i, MAC_ARRARY(pCurrent->spPrvCfg.id));

            i++;

 

            printf("scanACount = %d, scanGCount = %d\n", pScanResultSet->scanACount, pScanResultSet->scanGCount);

            printf("scanResult[ BSSID ] = "MAC_STR_FORMAT, MAC_ARRARY(pScanResult->BSSID));

            printf("scanResult[ SSID ]                = %s\n", pScanResult->SSID);

            printf("scanResult[ SSIDLength ]          = %d\n", pScanResult->SSIDLength);

            printf("scanResult[ channel ]             = %d\n", pScanResult->channel);

            printf("scanResult[ RSSI ]                = %d\n", pScanResult->RSSI);

            printf("scanResult[ auth ]                = %d\n", pScanResult->auth);

            printf("scanResult[ beaconInterval ]      = %d\n", pScanResult->beaconInterval);

            printf("scanResult[ DTIM ]                = %d\n", pScanResult->DTIM);

            printf("scanResult[ capInfo ]             = %d\n", pScanResult->capInfo);

            for(j = 0; j < SONICPOINT_SUPPORTRATE_MAX; j++)

                printf("scanResult[supRates %d] = %d\n", j, pScanResult->supRates[j]);

            for(j = 0; j < SONICPOINT_RATES_MAX; j++)

                printf("scanResult[rates %d] = %d\n", j, pScanResult->rates[j]);

        }

#endif

        printf("scan in G mode\n");

        pStatus = (SonicPointStatus*)&(pCurrent->spStatus);

        pScanResultSet = (SonicPointScanResultSet*)&(pStatus->spsScanResSet[1]);

        for(m_spCnt = 0; m_spCnt < pScanResultSet->scanGCount; m_spCnt++)

        {

            pScanResult = (SonicPointScanResult *)&(pScanResultSet->scanResult[m_spCnt]);

            printf("----------------------------------------------------\n");

            printf("[SP %d] mac "MAC_STR_FORMAT"\n", i, MAC_ARRARY(pCurrent->spPrvCfg.id));

            i++;

 

            printf("scanACount = %d, scanGCount = %d\n", pScanResultSet->scanACount, pScanResultSet->scanGCount);

            printf("scanResult[ BSSID ] = "MAC_STR_FORMAT, MAC_ARRARY(pScanResult->BSSID));

            printf("scanResult[ SSID ]                = %s\n", pScanResult->SSID);

            printf("scanResult[ SSIDLength ]          = %d\n", pScanResult->SSIDLength);

            printf("scanResult[ channel ]             = %d\n", pScanResult->channel);

            printf("scanResult[ RSSI ]                = %d\n", pScanResult->RSSI);

            printf("scanResult[ auth ]                = %d\n", pScanResult->auth);

            printf("scanResult[ beaconInterval ]      = %d\n", pScanResult->beaconInterval);

            printf("scanResult[ DTIM ]                = %d\n", pScanResult->DTIM);

            printf("scanResult[ capInfo ]             = %d\n", pScanResult->capInfo);

            for(j = 0; j < SONICPOINT_SUPPORTRATE_MAX && pScanResult->supRates[j]; j++)

                printf("scanResult[supRates %d] = %d\n", j, pScanResult->supRates[j]);

            for(j = 0; j < SONICPOINT_RATES_MAX && pScanResult->rates[j]; j++)

                printf("scanResult[rates %d]    = %d\n", j, pScanResult->rates[j]);

        }

        pCurrent = pCurrent->spNext;

    }

}

 

#endif
