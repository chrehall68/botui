#include "FileManagerWidget.h"

#include "ui_FileManagerWidget.h"

#include "MenuBar.h"
#include "RootController.h"
#include "FileUtils.h"
#include "AreYouSureDialog.h"
#include "NotYetImplementedDialog.h"
#include "Clipboard.h"
#include "FileActions.h"
#include "Device.h"
#include <QFileSystemModel>
#include <QFile>
#include <QDebug>
#include <iostream>
#include <fstream>

//#define HOME_PATH "/home/erin/Documents/KISS" // home programs folder for dev machine *CHANGE FOR YOUR SPECIFIC MACHINE*
#ifdef WOMBAT
#define HOME_PATH "/home/kipr/Documents/KISS"
#endif

FileManagerWidget::FileManagerWidget(Device *device, QWidget *parent)
	: QWidget(parent),
	  ui(new Ui::FileManagerWidget),
	  m_device(device),
	  m_menuBar(new MenuBar(this)),
	  m_fs(new QFileSystemModel(this)),
	  m_up(new QAction(tr("Go Up"), this))
{
	ui->setupUi(this);
	m_menuBar->setTitle(tr("File Manager"));
	layout()->setMenuBar(m_menuBar);
	QAction *ex = new QAction(tr("Exit"), this);
	connect(m_up, SIGNAL(triggered()), SLOT(up()));
	connect(ex, SIGNAL(triggered()), SLOT(exit()));
	m_menuBar->addAction(ex);
	m_menuBar->addAction(m_up);

	home();

	connect(ui->open, SIGNAL(clicked()), SLOT(open()));
	connect(ui->remove, SIGNAL(clicked()), SLOT(remove()));
	connect(ui->home, SIGNAL(clicked()), SLOT(home()));

	connect(ui->files->selectionModel(),
			SIGNAL(selectionChanged(QItemSelection, QItemSelection)),
			SLOT(selectionChanged(QItemSelection)));
}

FileManagerWidget::~FileManagerWidget()
{
	delete ui;
}

void FileManagerWidget::exit()
{
	RootController::ref().dismissWidget();
}

void FileManagerWidget::up()
{
	QModelIndexList indexes = ui->files->selectionModel()->selection().indexes();
	if (indexes.size() != 1)
		return;
	const QModelIndex newIndex = m_fs->parent(ui->files->rootIndex());
	if (!m_fs->filePath(newIndex).startsWith(HOME_PATH))
		return;
	ui->files->setRootIndex(newIndex);
	updateOptions();
}

void FileManagerWidget::open()
{
	QModelIndexList indexes = ui->files->selectionModel()->selection().indexes();
	if (indexes.size() != 1)
		return;

	// Open in File Manager if directory
	if (m_fs->isDir(indexes[0]))
	{
		ui->files->setRootIndex(indexes[0]);
		return;
	}

	// Use FileActions registery to execute file
	const QString ext = QFileInfo(m_fs->filePath(indexes[0])).absoluteFilePath();
	FileAction *action = FileActions::ref().action(ext);
	if (!action)
	{
		qWarning() << "No FileAction found for" << ext;
		return;
	}
	action->act(m_fs->filePath(indexes[0]), m_device);
}

void FileManagerWidget::copy()
{
	QModelIndexList indexes = ui->files->selectionModel()->selection().indexes();
	if (indexes.size() != 1)
		return;
	Clipboard::ref().setContents(m_fs->filePath(indexes[0]),
								 m_fs->isDir(indexes[0]) ? Clipboard::Folder : Clipboard::File);
}

void FileManagerWidget::paste()
{
	NotYetImplementedDialog::nyi();
	return;

	const QString from = Clipboard::ref().contents().toString();
	const QString to = m_fs->filePath(ui->files->rootIndex());
	const Clipboard::Type &type = Clipboard::ref().type();
	if (type == Clipboard::File || type == Clipboard::Folder)
	{
		if (QFileInfo(to).dir().exists(QFileInfo(from).fileName()))
		{
			AreYouSureDialog confirm;
			confirm.setDescription(tr("The directory %1 already exists. Pasting %2 will overwrite %1.")
									   .arg(QFileInfo(from).fileName())
									   .arg(from));
			int ret = RootController::ref().presentDialog(&confirm);
			if (ret == QDialog::Rejected)
				return;
			FileUtils::rm(QFileInfo(to).dir().filePath(QFileInfo(from).fileName()));
		}
		qDebug() << "Copy success?" << FileUtils::copy(from, to);
		return;
	}
	qWarning() << "FileManager doesn't know how to paste this type.";
}

void FileManagerWidget::remove()
{

	QModelIndexList indexes = ui->files->selectionModel()->selection().indexes();
	if (indexes.size() != 1)
		return;
	const QString to = m_fs->filePath(indexes[0]);

	if (QMessageBox::question(this, "Delete Folder?", tr("You are about to permanently delete %1. \n\nContinue?").arg(QFileInfo(to).fileName()), QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes)
		return;
	else
	{
		qDebug() << "Remove success?" << FileUtils::rm(to);
	}
	QStringList pathArguments;
	pathArguments << "/home/kipr/Documents/KISS/users.json";

	QProcess *myProcess = new QProcess(this);
	myProcess->start("cat", pathArguments);
	myProcess->waitForFinished();


	QByteArray output = myProcess->readAllStandardOutput();
	QByteArray output2 = output.sliced(1, output.size() - 2); //Just the users without begin/end {}
	QList<QByteArray> list = output2.split(',');
	QByteArray ba(QFileInfo(to).fileName().toLatin1());
	QByteArrayView y(ba);
	QString filename = "/home/kipr/Documents/KISS/users.json";

	QFile file(filename);
	file.open(QFile::ReadWrite | QFile::Text);
	file.resize(0); //empties the file 
	QTextStream stream(&file);
	stream << "{";
	for (int i = 0; i < list.length(); i++)
	{
	
		if ((list[i].indexOf(y) == -1))
		{

			stream << list[i];
			if (i < list.length() - 1)
			{
				stream << ",";
			}
		}
	}
	stream << "}";
}

void FileManagerWidget::home()
{

	QDir homeDir = QDir::homePath();

	homeDir.setPath(HOME_PATH);

	ui->files->setModel(this->m_fs);
	qDebug() << "homeDir.path(): " << homeDir.path();
	m_fs->setRootPath(homeDir.path());
	m_fs->setFilter(QDir::Dirs | QDir::Drives | QDir::NoDotAndDotDot | QDir::AllDirs);
	ui->files->setRootIndex(m_fs->index(homeDir.path()));
}

void FileManagerWidget::selectionChanged(const QItemSelection &selected)
{
	QModelIndexList indexes = selected.indexes();
	if (indexes.size() != 1)
		return;
	QModelIndex index = indexes[0];
	const QString ext = QFileInfo(m_fs->filePath(index)).absoluteFilePath();
	FileAction *action = FileActions::ref().action(ext);
	ui->open->setEnabled(m_fs->isDir(index) || action);
	ui->open->setText(tr("Open"));
}

void FileManagerWidget::updateOptions()
{
	// m_up->setEnabled(ui->files->rootIndex() != m_fs->index(m_fs->rootPath()));
}