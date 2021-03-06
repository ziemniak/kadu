#ifndef MPRISCONTROLLER_H
#define MPRISCONTROLLER_H

#include <QtCore/QObject>
#include <QtCore/QVariantMap>

struct PlayerStatus
{
	int i1;
	int i2;
	int i3;
	int i4;
};
Q_DECLARE_METATYPE(PlayerStatus);

struct TrackInfo
{
	QString title;
	QString artist;
	QString album;
	QString track;
	QString file;
	uint    time;
};
Q_DECLARE_METATYPE(TrackInfo);

class MPRISController : public QObject
{
	Q_OBJECT

	PlayerStatus currentStatus_;
	TrackInfo    currentTrack_;
	bool         active_;
	QString      service;

public:
	MPRISController(QString service);
	~MPRISController();

	PlayerStatus currentStatus() { return currentStatus_; };
	TrackInfo    currentTrack()  { return currentTrack_;  };
	bool         active()        { return active_;        };

public slots:
	void statusChanged(PlayerStatus status);
	void trackChanged(QVariantMap track);
};

#endif
