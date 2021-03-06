#ifndef KADU_VOICE_H
#define KADU_VOICE_H

#include <QtGui/QDialog>
#include <QThread>
#include <QMutex>
#include <QSemaphore>
#include <QtCore/QList>

extern "C" {
	#include "libgsm/inc/gsm.h"
};

#include "../dcc/dcc.h"
#include "../dcc/dcc_handler.h"
#include "../sound/sound.h"

#include "main_configuration_window.h"

class ActionDescription;
class ChatWidget;
class PlayThread;
class RecordThread;

class QCheckBox;

/**
 * @defgroup voice Voice
 * @{
 */
struct gsm_sample
{
	char *data;
	int length;
};

class VoiceChatDialog : public QDialog, public DccHandler
{
	Q_OBJECT

		static QList<VoiceChatDialog *> VoiceChats;
		DccSocket* Socket;

	public:
		VoiceChatDialog();
		~VoiceChatDialog();

		bool addSocket(DccSocket *socket);
		void removeSocket(DccSocket *socket);

		int dccType() { return GG_SESSION_DCC_VOICE; }

		bool socketEvent(DccSocket *socket, bool &lock);

		void connectionDone(DccSocket *socket) {}
		void connectionError(DccSocket *socket) {}

		void connectionAccepted(DccSocket *socket) {}
		void connectionRejected(DccSocket *socket) {}

		static void destroyAll();
		static void sendDataToAll(char *data, int length);

		void sendData(char *data, int length);

		bool chatFinished;
};

class VoiceManager : public ConfigurationUiHandler, public DccHandler
{
	Q_OBJECT
		ActionDescription *voiceChatActionDescription;
		MessageBox *GsmEncodingTestMsgBox;
		SoundDevice GsmEncodingTestDevice;
		gsm GsmEncodingTestHandle;
		int16_t *GsmEncodingTestSample;
		gsm_frame *GsmEncodingTestFrames;
		int GsmEncodingTestCurrFrame;
		SoundDevice device;

		PlayThread *playThread;
		RecordThread *recordThread;
		gsm voice_enc;
		gsm voice_dec;

		void resetCoder();
		void resetDecoder();
		bool askAcceptVoiceChat(DccSocket *socket);

		QCheckBox *testFast;
		QCheckBox *testCut;

		void createDefaultConfiguration();

		void makeVoiceChat(UinType dest);

	private slots:
		void voiceChatActionActivated(QAction *sender, bool toggled);

		void testGsmEncoding();
		void gsmEncodingTestSampleRecorded(SoundDevice device);
		void gsmEncodingTestSamplePlayed(SoundDevice device);
		void playGsmSampleReceived(char *data, int length);
		void recordSampleReceived(char *data, int length);
		void mainDialogKeyPressed(QKeyEvent *e);
		void chatKeyPressed(QKeyEvent *e, ChatWidget *chatWidget, bool &handled);

		void chatCreated(ChatWidget *chat);
		void chatDestroying(ChatWidget *chat);

	public:
		VoiceManager();
		virtual ~VoiceManager();

		int setup();
		void free();
		void resetCodec();
		void addGsmSample(char *data, int length);

		bool addSocket(DccSocket *socket);
		void removeSocket(DccSocket *socket) {}

		int dccType() { return GG_SESSION_DCC_VOICE; }

		bool socketEvent(DccSocket *socket, bool &lock);

		void connectionDone(DccSocket *socket) {}
		void connectionError(DccSocket *socket) {}

		void connectionAccepted(DccSocket *socket) {}
		void connectionRejected(DccSocket *socket) {}

		virtual void mainConfigurationWindowCreated(MainConfigurationWindow *mainConfigurationWindow);

};

class PlayThread : public QThread
{
	Q_OBJECT

		QSemaphore *wsem;
		void waitForData(); //czeka na nowe dane
		void moreData(); //daje zna�, �e s� nowe dane

		QList<struct gsm_sample> samples;
		QMutex samplesMutex; // chroni dost�p do samples

		bool end;
//		QMutex endMutex; //chroni dost�p do end

	public:
		PlayThread();
		~PlayThread();
		void run();
		void endThread();
		void addGsmSample(char *data, int length);

	signals:
		void playGsmSample(char *data, int length);


};

class RecordThread : public QThread
{
	Q_OBJECT

		bool end;
//		QMutex endMutex; //chroni dost�p do end
	public:
		RecordThread();
		void run();
		void endThread();

	signals:
		void recordSample(char *data, int length);


};

extern VoiceManager *voice_manager;

/** @} */

#endif
