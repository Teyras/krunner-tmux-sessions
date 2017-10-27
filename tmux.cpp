#include <iostream>

#include <KLocalizedString>
#include <KConfigCore/KConfig>
#include <KConfigCore/KConfigGroup>

#include <QApplication>
#include <QProcess>

#include "tmux.h"

using namespace std;

Tmux::Tmux(QObject *parent, const QVariantList &args)
    : Plasma::AbstractRunner(parent, args)
{
    Q_UNUSED(args);
    
    // General runner configuration
    setObjectName(QLatin1String("Tmux"));
    setHasRunOptions(true);
    setIgnoredTypes(Plasma::RunnerContext::Directory |
                    Plasma::RunnerContext::File |
                    Plasma::RunnerContext::NetworkLocation);
    setSpeed(AbstractRunner::NormalSpeed);
    setPriority(HighestPriority);
    setDefaultSyntax(
        Plasma::RunnerSyntax(
            QString::fromLatin1(":q:"),
            i18n("Opens a terminal window attached to a tmux session")
        )
    );
}

Tmux::~Tmux() {}

void Tmux::match(Plasma::RunnerContext &context)
{
    if (!context.isValid()) return;

    QString query = context.query();
    bool isQueryPrefixed = false;
    if (query.startsWith("tmux")) {
	    query = query.right(query.length() - 4).trimmed();
	    isQueryPrefixed = true;
    }

    QList<Plasma::QueryMatch> matches;

    QProcess ls;
    ls.start("tmux", QStringList() << "ls");
    ls.waitForFinished();

    while (ls.canReadLine()) {
        QString line = QString::fromLocal8Bit(ls.readLine());
        QString sessionName = line.split(':').first();
        
        if (sessionName.startsWith(query)) {
            // We have a match
            Plasma::QueryMatch match(this);
            // Basic properties for the match
            match.setIcon(QIcon::fromTheme("utilities-terminal"));
            match.setText(sessionName);
            match.setSubtext(i18n("Attach to tmux session"));
            match.setData(sessionName);
            match.setId(sessionName);
	    float relevance = (float) query.length() / (float) sessionName.length();
            match.setRelevance(isQueryPrefixed ? 1.0 : relevance);
            matches.append(match);
        }
    }

    context.addMatches(matches);
}

void Tmux::run(const Plasma::RunnerContext &context, const Plasma::QueryMatch &match)
{
    Q_UNUSED(context);

    QString program("konsole");
    QStringList args;
    args.append("-e");
    args.append("tmux");
    args.append("attach");
    args.append("-t");
    args.append(match.data().toString());

    QProcess::startDetached(program, args);
}

K_EXPORT_PLASMA_RUNNER(tmux, Tmux)

#include "tmux.moc"
