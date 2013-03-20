#ifndef _CAMERAINPUTMANAGER_H_
#define _CAMERAINPUTMANAGER_H_

#include <QObject>
#include <QTimer>
#include "Singleton.h"
#include <kovan/camera.hpp>

class CameraInputManager : public QObject, public Singleton<CameraInputManager>
{
Q_OBJECT
	
public:
	CameraInputManager();
	
	void setInputProvider(Camera::InputProvider *inputProvider);
	Camera::InputProvider *inputProvider() const;
	
	void setFrameRate(int frameRate);
	int frameRate() const;
	
	void setWidth(const unsigned width);
	void setHeight(const unsigned height);
	
	bool retain();
	bool release();
	
	bool isOpen() const;
	
	cv::Mat image() const;
	
signals:
	void frameAvailable(const cv::Mat &image);

public slots:
	void updateCamera();
	
private:
	Camera::InputProvider *m_inputProvider;
	QTimer *m_timer;
	cv::Mat m_image;
	int m_frameRate;
	int m_refs;
};

class CameraInputAdapter : public Camera::InputProvider
{
public:
	CameraInputAdapter(CameraInputManager *manager);
	~CameraInputAdapter();
		
	virtual bool open(const int number);
	virtual bool isOpen() const;
	virtual void setWidth(const unsigned width);
	virtual void setHeight(const unsigned height);
	virtual bool next(cv::Mat &image);
	virtual bool close();
	
private:
	CameraInputManager *m_manager;
};

#endif
