#ifndef SINOVOICEASRSERVICE_H
#define SINOVOICEASRSERVICE_H

#include "ASRServiceAbs.h"
#include "hci_asr_recorder.h"
#include "CommonTool.h"
#include "AccountInfo.h"

class SinoVoiceASRService : public ASRServiceAbs
{
public:
    typedef enum _tag_AsrRecogType
    {
        kRecogTypeUnkown = -1,      //未知类型
        kRecogTypeCloud = 0,        //云端识别
        kRecogTypeLocal,            //本地识别
    }AsrRecogType;

    typedef enum _tag_AsrRecogMode
    {
        kRecogModeUnkown = -1,      //未知类型
        kRecogModeFreetalk = 0,     //自由说
        kRecogModeGrammar,          //语法识别
    }AsrRecogMode;

    SinoVoiceASRService();
    virtual ~SinoVoiceASRService();

    virtual bool startService();
    virtual bool stopService();
    virtual std::string errorString();

    void handleRecordEventChange(RECORDER_EVENT eRecorderEvent);
    void handleRecorderRecogFinish(RECORDER_EVENT eRecorderEvent,ASR_RECOG_RESULT *psAsrRecogResult);
    void handleRecorderRecogProcess(RECORDER_EVENT eRecorderEvent,ASR_RECOG_RESULT *psAsrRecogResult);
    void handleRecorderErr(RECORDER_EVENT eRecorderEvent,HCI_ERR_CODE eErrorCode);
    void handleRecorderRecordingCallback(unsigned char * pVoiceData,unsigned int uiVoiceLen);

private:
    bool CheckAndUpdataAuth();
    void GetCapkeyProperty(const string&cap_key,AsrRecogType & type,AsrRecogMode &mode);
    bool Init();
    bool Uninit(void);

    void EchoGrammarData(const string &grammarFile);

    static void HCIAPI RecordEventChange(RECORDER_EVENT eRecorderEvent, void *pUsrParam);

    static void HCIAPI RecorderRecogFinish(
        RECORDER_EVENT eRecorderEvent,
        ASR_RECOG_RESULT *psAsrRecogResult,
        void *pUsrParam);

    static void HCIAPI RecorderRecogProcess(
        RECORDER_EVENT eRecorderEvent,
        ASR_RECOG_RESULT *psAsrRecogResult,
        void *pUsrParam);

    static void HCIAPI RecorderErr(
        RECORDER_EVENT eRecorderEvent,
        HCI_ERR_CODE eErrorCode,
        void *pUsrParam);

    static void HCIAPI RecorderRecordingCallback(
        unsigned char * pVoiceData,
        unsigned int uiVoiceLen,
        void * pUsrParam
        );

private:
    std::string m_error;
    AsrRecogType m_RecogType;
    AsrRecogMode m_RecogMode;
    unsigned int m_GrammarId;
};

#endif // SINOVOICEASRSERVICE_H
