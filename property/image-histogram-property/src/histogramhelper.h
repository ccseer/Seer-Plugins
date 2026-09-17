#pragma once

#include <QString>
#include <QStringList>

int runImageHistogram(const QStringList &arguments);
int executeImageHistogram(const QString &inputPath, const QString &outputBasePath, const QString &outputDirPath);
