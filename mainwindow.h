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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QList>
#include <QMainWindow>
#include <QSettings>
#include <QDebug>

#include "runnable.h"

namespace Ui
{
	class MainWindow;
}

class MainWindow : public QMainWindow
{
	Q_OBJECT
public:
	MainWindow(QWidget *parent = 0);
	~MainWindow();

//	friend class Runnable;

protected:
	void changeEvent(QEvent *e);

private:
	Ui::MainWindow *ui;
	QSettings m_profiles;
	QList<Runnable*> m_runnables;
	bool m_ok;

	void loadProfiles(bool load_default = true);
	void checkProcessing();

private slots:
	void addFiles();
	void forceExtensionChange(bool force);
	void removeSelectedFiles();

	void removeSelectedProfile();
	void loadSelectedProfile();
	void saveProfile();

	void runJobs();
	void stopJobs();
	void killJobs();

	void setGuiProcessingState(bool processing);

	void removeRunnable(Runnable* runnable, bool ok = true);

	void startedJob(QString cmd);
	void doneJob(QString cmd, QString file, int retcode);
	void failedJob(QString error);
};

#endif // MAINWINDOW_H
