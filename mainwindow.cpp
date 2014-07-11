/* QtJobs - a user-friendly way of multitasking
Copyright (C) 2010  David Watzke <david@watzke.cz>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>. */

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QtGui>
#include <QtWidgets>
#include <QThreadPool>

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow), m_killed(false)
{
	ui->setupUi(this);

	/* make sure the first tab is activated */
	ui->tabWidget->setCurrentIndex(0);

	ui->threadBox->setValue(QThread::idealThreadCount());

	/* destroy the threads inactive for more than 100 ms;
	   to clarify: the default of 30 sec causes trouble when
	   you suddenly decide that you want to lower the threadcount;
	   newly set threadcount is ignored and old inactive threads are reused */
	QThreadPool::globalInstance()->setExpiryTimeout(100);

	loadProfiles();
}

void MainWindow::addFiles()
{
	QStringList files = QFileDialog::getOpenFileNames(this,
				tr("Select files to be processed"),
				QDir::homePath());
	ui->fileList->addItems(files);
}

void MainWindow::removeSelectedFiles()
{
	QList<QListWidgetItem*> items = ui->fileList->selectedItems();

	ui->fileList->setUpdatesEnabled(false);

	while(!items.isEmpty())
		delete items.takeLast();

	ui->fileList->setUpdatesEnabled(true);
}

void MainWindow::loadProfiles(bool load_default)
{
	while(ui->comboProfile->count())
		ui->comboProfile->removeItem(0);

	QStringList profiles = m_settings.childGroups();

	if(!profiles.contains("default"))
	{
		m_settings.beginGroup("default");
		m_settings.setValue("name", tr("Default"));
		m_settings.setValue("command", QString());
		m_settings.setValue("extension", QString("ext"));
		m_settings.setValue("outdir", QDir::homePath());
		m_settings.setValue("samedir", false);
		m_settings.setValue("want_extension", false);
		m_settings.setValue("autothread", true);
		m_settings.setValue("threadcount", QThread::idealThreadCount());
		m_settings.endGroup();

		profiles << "default";
	}

	foreach(QString profile, profiles)
	{
		m_settings.beginGroup(profile);
		QString name = m_settings.value("name").toString();
		m_settings.endGroup();

		if(name.isEmpty())
			continue;

		//qDebug() << "adding profile" << profile << "as" << name;

		ui->comboProfile->addItem(name, profile);
	}

	if(load_default)
	{
		int index = ui->comboProfile->findData("default");
		ui->comboProfile->setCurrentIndex(index);

		loadSelectedProfile();
	}
}

void MainWindow::removeSelectedProfile()
{
	int index = ui->comboProfile->currentIndex();
	QString profile = ui->comboProfile->itemData(index).toString();

	if(profile.isEmpty())
		return;

	if(profile == "default")
	{
		QMessageBox::critical(this, tr("Error!"),
				      tr("You cannot remove the default profile!"));
		return;
	}

	ui->comboProfile->removeItem(index);

	index = ui->comboProfile->findData("default");
	ui->comboProfile->setCurrentIndex(index);

	m_settings.remove(profile);
}

void MainWindow::loadSelectedProfile()
{
	int index = ui->comboProfile->currentIndex();
	QString profile = ui->comboProfile->itemData(index).toString();

	Q_ASSERT(!profile.isEmpty());

	m_settings.beginGroup(profile);
	QString command = m_settings.value("command").toString();
	QString extension = m_settings.value("extension", "ext").toString();
	QString outdir = m_settings.value("outdir", QDir::homePath()).toString();
	bool samedir = m_settings.value("samedir", false).toBool();
	bool want_extension = m_settings.value("want_extension", false).toBool();
	bool autothread = m_settings.value("autothread", true).toBool();
	int threadcount = m_settings.value("threadcount",
					   QThread::idealThreadCount()).toInt();
	QString name = "";
	if(profile != "default")
		name = m_settings.value("name").toString();
	m_settings.endGroup();

	ui->commandEdit->setText(command);

	if(!want_extension)
		ui->extCb->setChecked(false);
	else
	{
		ui->extCb->setChecked(true);
		ui->extEdit->setText(extension);
	}

	if(samedir)
		ui->radioSame->setChecked(true);
	else
	{
		ui->radioOther->setChecked(true);
		ui->outDirEdit->setText(outdir);
	}

	//ui->threadcountCb->setChecked(!autothread);
	if(autothread)
		ui->idealRadio->setChecked(true);
	else
		ui->threadcountRadio->setChecked(true);
	ui->threadBox->setValue(threadcount);

	ui->profileEdit->setText(name);
}

void MainWindow::saveProfile()
{
	QString profile = ui->profileEdit->text();

	if(profile.isEmpty())
	{
		int index = ui->comboProfile->currentIndex();
		profile = ui->comboProfile->itemData(index).toString();
	}

	Q_ASSERT(!profile.isEmpty());

	m_settings.beginGroup(profile);
	m_settings.setValue("name", profile);
	m_settings.setValue("command", ui->commandEdit->text());
	m_settings.setValue("extension", ui->extEdit->text());
	m_settings.setValue("outdir", ui->outDirEdit->text());
	m_settings.setValue("samedir", ui->radioSame->isChecked());
	m_settings.setValue("want_extension", ui->extCb->isChecked());
	m_settings.setValue("autothread", !ui->threadcountRadio->isChecked());
	m_settings.setValue("threadcount", ui->threadBox->value());
	m_settings.endGroup();

	loadProfiles(false);

	int index = ui->comboProfile->findData(profile);
	ui->comboProfile->setCurrentIndex(index);

	loadSelectedProfile();

	//ui->profileEdit->setText("");
}

void MainWindow::chooseDirectory()
{
	QString dir = QFileDialog::getExistingDirectory(this,
				tr("Choose an output directory"), QDir::homePath());

	if(!dir.isEmpty())
		ui->outDirEdit->setText(dir);
}

void MainWindow::runJobs()
{
	/* reset the widget state */
	ui->commandList->clear();
	ui->processedList->clear();
	ui->errorList->clear();
	ui->progressBar->reset();

	QStringList fileList;
	int count = ui->fileList->count();

	for(int i=0; i < count; ++i)
		fileList << ui->fileList->item(i)->text();

	if(count < 1)
	{
		QMessageBox::critical(this, tr("Error!"),
			tr("Please add some files to the file list on the \"File "
			   "list\" tab."));
		return;
	}

	if(ui->extCb->isChecked() && ui->extEdit->text().isEmpty())
	{
		QMessageBox::critical(this, tr("Error!"),
			tr("Please either enter an extension or uncheck the option "
			   "for it's changing."));
		return;
	}

	if(ui->radioOther->isChecked())
	{
		QFileInfo dirInfo(ui->outDirEdit->text());
		if(!dirInfo.isDir() || !dirInfo.isWritable())
		{
			QMessageBox::critical(this, tr("Error!"),
				tr("Please select a valid writable output directory."));
			return;
		}
	}

	if(!ui->commandEdit->text().contains("$FILE"))
	{
		QMessageBox::critical(this, tr("Error!"),
			tr("The command must contain the $FILE variable (and should "
			   "also contain the optional $OFILE variable)."));
		return;
	}

	ui->progressBar->setRange(0, count);
	ui->progressBar->setFormat("%v/%m (%p%)");
	ui->progressBar->setValue(0);

	int threadcount = QThread::idealThreadCount();

	if(ui->threadcountRadio->isChecked())
		threadcount = ui->threadBox->value();

	QThreadPool::globalInstance()->setMaxThreadCount(threadcount);

	setGuiProcessingState(true);

	foreach(QString fileName, fileList)
	{
		RunnableSettings s;

		s.command = ui->commandEdit->text();
		s.extension = ui->extEdit->text();
		s.outdir = ui->outDirEdit->text();
		s.infile = fileName;
		s.bExtension = ui->extCb->isChecked();
		s.bSameDir = ui->radioSame->isChecked();

		Runnable *runnable = new Runnable(s);
		connect(runnable, SIGNAL(destroyMe(Runnable*,bool)),
			this, SLOT(removeRunnable(Runnable*,bool)));
		connect(runnable, SIGNAL(started(QString)),
			this, SLOT(startedJob(QString)));
		connect(runnable, SIGNAL(done(QString,QString,int)),
			this, SLOT(doneJob(QString,QString,int)));
		connect(runnable, SIGNAL(failed(QString)),
			this, SLOT(failedJob(QString)));

		m_runnables << runnable;

		QThreadPool::globalInstance()->start(runnable);
	}

	ui->tabWidget->setCurrentIndex(2);
}

void MainWindow::stopJobs()
{
	foreach(Runnable *runnable, m_runnables)
		runnable->stop();
}

void MainWindow::killJobs()
{
	foreach(Runnable *runnable, m_runnables)
		runnable->kill();
}

void MainWindow::removeRunnable(Runnable *runnable, bool ok)
{
	m_runnables.removeAll(runnable);
	delete runnable;

	m_killed = !ok;

	checkProcessing();
}

void MainWindow::startedJob(QString cmd)
{
	new QListWidgetItem(cmd, ui->commandList);
}

void MainWindow::doneJob(QString cmd, QString file, int retcode)
{
	int done = ui->progressBar->value()+1;
	ui->progressBar->setValue(done);

	if(!retcode)
		ui->processedList->addItem(file);
	else
	{
		QString error = QString(tr("command returned %1: %2"))
					.arg(retcode).arg(cmd);
		ui->errorList->addItem(error);
	}

	QList<QListWidgetItem*> item = ui->commandList->findItems(cmd,
						Qt::MatchExactly|
						Qt::MatchCaseSensitive);

	if(item.count() != 1)
	{
		qDebug() << "WARNING: removeCommandFromList() found >1 item."
			 << "This shouldn't happen. Items:";
		foreach(QListWidgetItem* it, item)
			qDebug() << "> " << it->text();
	}

	if(item.count() > 0)
		delete item.at(0);
	else
		qDebug() << "BUG: removeCommandFromList() found no items."
			 << "This should never happen.";
}

void MainWindow::failedJob(QString error)
{
	int done = ui->progressBar->value()+1;
	ui->progressBar->setValue(done);

	ui->errorList->addItem(error);
}

void MainWindow::setGuiProcessingState(bool processing)
{
	ui->runBtn->setDisabled(processing);
	ui->stopBtn->setEnabled(processing);
	ui->killBtn->setEnabled(processing);

	if(processing)
		m_killed = false;
}

void MainWindow::checkProcessing()
{
	if(m_runnables.count())
		return;

	/* looks like we're done processing */

	if(QThreadPool::globalInstance()->activeThreadCount() != 0)
		qDebug() << "looks like processing is done but"
			 << "QThreadPool::activeThreadCount() != 0"
			 << "(but it's not that weird)";

	setGuiProcessingState(false);

	ui->commandList->clear();
	//ui->processedList->clear();

	if(!m_killed && !ui->errorList->count())
		QMessageBox::information(this, tr("Done!"),
			tr("Done! All the files were successfully processed!"));
	else if(!m_killed)
	{
		QMessageBox::StandardButton btn;
		btn = QMessageBox::warning(this, tr("Done with errors!"),
				tr("Done with errors! See the list of the failed "
				   "commands on the \"Progress\" tab."));
		if(btn == QMessageBox::Ok)
			ui->tabWidget->setCurrentIndex(2);
	}
	else
		QMessageBox::critical(this, tr("Interrupted!"),
			tr("Processing was interrupted by pressing the Stop or "
			   "the KILL button."));

	ui->progressBar->reset();
}

void MainWindow::forceExtensionChange(bool force)
{
	if(force)
		ui->extCb->setChecked(true);

	ui->extCb->setDisabled(force);
}

MainWindow::~MainWindow()
{
	delete ui;
}

void MainWindow::changeEvent(QEvent *e)
{
	QMainWindow::changeEvent(e);

	switch(e->type())
	{
	case QEvent::LanguageChange:
		ui->retranslateUi(this);
		break;
	default:
		break;
	}
}
