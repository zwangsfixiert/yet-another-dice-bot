#ifndef PROFILE_HPP
#define PROFILE_HPP

#include <QtWidgets>
#include <QString>
#include <QList>

struct Profile
{
    Profile(QString username, QString password, QString accesstoken, bool chatenabled, bool autoload) :
        username(username), password(password), accesstoken(accesstoken), chatenabled(chatenabled), autoload(autoload) {
    }

    QString username;
    QString password;
    // TODO support 2FA
    QString accesstoken;

    bool chatenabled;
    bool autoload;
    bool isRunning{false};
    bool isLoaded{false};
};

class ProfileManager {
public:
    ProfileManager() {}

    bool LoadProfiles() {
        return true;
    }

    bool SaveProfiles() {
        return true;
    }

    bool AddProfile(QString username, QString password, QString token, bool allowChat, bool autoLoad) {
        Profile profile(username, password, token, allowChat, autoLoad);
        return AddProfile(profile);
    }

    bool AddProfile(Profile& profile) {
        bool found{false};
        for(auto& profileEntry : profiles) {
            if(profileEntry.username == profile.username)
                found = true;
        }
        if(!found) {
            profiles.push_back(profile);
        }

        return !found;
    }

    bool RemoveProfile(QString username) {
        QMutableListIterator<Profile> it(profiles);
        while (it.hasNext()) {
            if (it.next().username == username) {
                it.remove();
                return true;
            }
        }

        return false;
    }

    QList<Profile>& GetProfiles()
    {
        return profiles;
    }

    Profile* GetProfile(QString username) {
        for(auto& profile : profiles) {
            if(profile.username == username)
                return &profile;
        }
        return nullptr;
    }

private:
    QList<Profile> profiles;
};

#endif // PROFILE_HPP
