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

#ifndef RUNNABLE_H
#define RUNNABLE_H

#include <QObject>
#include <QRunnable>
#include <QString>

// struktura s informacemi pro Runnable
struct RunnableSettings
{
	QString command;
	QString extension;
	QString outdir;
	QString infile;
	//QString outfile;
	bool bExtension;
	bool bSameDir;
};

// chceme ze třídy vysílat signály, proto ji zakládáme i na QObjectu
class Runnable : public QObject, public QRunnable
{
	// nezapomeneme na makro Q_OBJECT
	Q_OBJECT
public:
	Runnable(const RunnableSettings &s);

	void stop() { m_stop = true; }
	void kill() { stop(); m_kill = true; }

protected:
	void run();

private:
	QString m_command;
	QString m_infile;

	bool m_stop;
	bool m_kill;

signals:
	void started(QString);
	void done(QString,QString,int);
	void failed(QString);
	void destroyMe(Runnable*,bool);
};

#endif // RUNNABLE_H
