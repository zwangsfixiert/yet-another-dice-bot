#ifndef SCRIPTMANAGER_HPP
#define SCRIPTMANAGER_HPP

#include <QJSEngine>

class ScriptManager
{
public:
    ScriptManager();
    ~ScriptManager();

    QJSValue RunScript(QString source, QString file, int line);

private:
    QJSEngine engine;
    QList<QJSValue> scripts;
};

#endif // SCRIPTMANAGER_HPP
