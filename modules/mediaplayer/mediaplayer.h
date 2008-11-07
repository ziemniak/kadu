#ifndef MEDIA_PLAYER_H
#define MEDIA_PLAYER_H

#include "configuration_aware_object.h"
#include "modules.h"
#include "misc.h"
#include "main_configuration_window.h"

class QTimer;

class ChatWidget;
class CustomInput;
class PlayerCommands;
class PlayerInfo;
class ToolBar;
class ToolButton;
class UserGroup;

class MediaPlayerStatusChanger;

class MediaPlayer : public ConfigurationUiHandler, ConfigurationAwareObject
{
	Q_OBJECT

	MediaPlayerStatusChanger *mediaPlayerStatusChanger;
	PlayerInfo *playerInfo;
	PlayerCommands *playerCommands;

	ActionDescription *enableMediaPlayerStatuses;
	ActionDescription *mediaPlayerMenu;

	QTimer *timer;
	QString currentTitle;
	QMenu *menu;
	int popups[6];
	bool winKeyPressed; // TODO: this is lame, make it good ;)
	QMap<ChatWidget *, QPushButton *> chatButtons;

	int pos();

	/**
		Returns Chat window, which currenly has a focus.
	*/
	ChatWidget *getCurrentChat();

	/**
		Returns true if playerInfo is supported by 3rd party
		player module which is currently loaded (if any).
	*/
	bool playerInfoSupported();

	/**
		Returns true if playerCommands is supported by 3rd party
		player module which is currently loaded (if any).
	*/
	bool playerCommandsSupported();

	void setControlsEnabled(bool enabled);

	void createDefaultConfiguration();

private slots:
	// Proxy methods for 3rd party modules
	void nextTrack();
	void prevTrack();
	void play();
	void stop();
	void pause();
	void setVolume(int vol);
	void incrVolume();
	void decrVolume();
	QString getPlayerName();
	QString getPlayerVersion();
	QString getTitle(int position = -1);
	QString getAlbum(int position = -1);
	QString getArtist(int position = -1);
	QString getFile(int position = -1);
	int getLength(int position = -1);
	int getCurrentPos();
	bool isPlaying();
	bool isActive();
	QStringList getPlayListTitles();
	QStringList getPlayListFiles();
	uint getPlayListLength();

	/*
		Applies all needed functions to newly created Chat window.
	*/
	void chatWidgetCreated(ChatWidget *);

	/*
		Removes all needed functions from Chat window being destroyed.
	*/
	void chatWidgetDestroying(ChatWidget *);
	void checkTitle();
	void chatKeyPressed(QKeyEvent *, CustomInput *, bool &);
	void chatKeyReleased(QKeyEvent *, CustomInput *, bool &);

	void mediaPlayerStatusChangerActivated(QAction *sender, bool toggled);
	void mediaPlayerMenuActivated(QAction *sender, bool toggled);

protected:
	void configurationUpdated();

public:
	MediaPlayer(bool firstLoad);
	~MediaPlayer();

	/*
		Looks for special tags in string 'str' and replaces it by
		assigned values. Then returns string in new form.

		Tag:		Replaces by:
		%t			Song title
		%a			Album title
		%r			Artist
		%f			File name
		%l			Length in format MM:SS
		%c			Current playing position in format MM:SS
		%p			Current playing position in percents
		%d			Current user description (if any)
		%n			Player name
		%v			Player version.
	*/
	QString parse(const QString &str);

	/*
		Returns formatted string of track length, which should be
		value returned by getLength() or getCurrentPosition() method.
	*/
	QString formatLength(int length);

	/*
		3rd party player module has to call this method in its constructor to register itself
		in MediaPlayer module so everything work.

		If one of PlayweInfo or PlayerCommands isn't supported by 3rd party module,
		just pass null (0) value for argument in this method.

		The method returns true if registering has successed, or false if there is already
		other module registered.
	*/
	bool registerMediaPlayer(PlayerInfo *info, PlayerCommands *cmds);

	/*
		3rd party player module has to call this method in its destructor to unregister itself
		from MediaPlayer module so other module can be registered.
	*/
	void unregisterMediaPlayer();

	virtual void mainConfigurationWindowCreated(MainConfigurationWindow *mainConfigurationWindow);

public slots:
	/**
		Puts song title into current chat edit field.
	*/
	void putSongTitle(int);

	/**
		Puts whole playlist into current chat edit field.
	*/
	void putPlayList(int);

	/**
		Shows currently played title in hint (Pseudo-OSD).
	*/
	void putTitleHint(QString title);

};

extern MediaPlayer *mediaplayer;

#endif