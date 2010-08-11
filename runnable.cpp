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

#include "runnable.h"

//#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QProcess>
#include <QRegExp>

Runnable::Runnable(const RunnableSettings &s)
	: m_stop(false), m_kill(false)
{
	/* we handle this by ourselves as it gives us more freedom */
	setAutoDelete(false);

	m_infile = s.infile;
	m_command = s.command;

	QFileInfo fileInfo(m_infile);
	QString outfile;

	if(s.bSameDir)
		outfile = fileInfo.dir().canonicalPath() + "/";
	else
		outfile = s.outdir + "/";

	QString fileName = fileInfo.fileName();

	if(s.bExtension)
	{
		QRegExp regex("^(.*)\\.(\\w+)$", Qt::CaseInsensitive,
						 QRegExp::RegExp2);
		fileName.replace(regex, "\\1." + s.extension);
	}

	outfile += fileName;

	m_command.replace("$FILE", "\""+m_infile+"\"");
	m_command.replace("$OFILE", "\""+outfile+"\"");
}

void Runnable::run()
{
	if(m_stop)
	{
		//qDebug() << "user pressed Stop or KILL, so not even starting";
		emit destroyMe(this, false);
		return;
	}

	//qDebug() << this << "about to run" << m_command;

	QProcess process;

	process.start(m_command, QIODevice::ReadWrite|QIODevice::Text);

	if(process.waitForStarted(1000))
	{
		emit started(m_command);
	}
	else
	{
		QString errorString = tr("command failed to start");

		if(process.error() != QProcess::FailedToStart)
			errorString = tr("command failed due to an unknown error");

		emit failed(errorString + ": " + m_command);

		emit destroyMe(this, true);

		return;
	}

	/* loop checking whether the "KILL" button has been pressed (every 200 ms) */
	while(m_kill || !process.waitForFinished(200))
	{
		if(!m_kill)
			continue;

		//qDebug() << "killing" << m_command;

		process.kill();
		break;
	}

	process.waitForFinished(1000);

	/* this condition is not right but it should be fixed if/when
	   I which to using QProcess signals */
	emit done(m_command, m_infile, m_kill ? 137 : process.exitCode());

	emit destroyMe(this, !m_kill);
}
