#ifndef WINDOWSAUDIOSESSIONCONTROLLER_H
#define WINDOWSAUDIOSESSIONCONTROLLER_H

#include <windows.h>
#include <QSet>
#include <QString>

namespace Saiko {
namespace Adapters {

class WindowsAudioSessionController {
public:
    static bool setAbsoluteMuteExcept(const QSet<QString>& exemptExes, bool muteActive);
};

}
}

#endif // WINDOWSAUDIOSESSIONCONTROLLER_H
