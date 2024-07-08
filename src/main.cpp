#include <Geode/Geode.hpp>

using namespace geode::prelude;

#include <Geode/modify/LevelInfoLayer.hpp>
#include <Geode/modify/EditLevelLayer.hpp>
#include <Geode/ui/Notification.hpp>
#include <Geode/modify/GJAccountManager.hpp>
#include <Geode/modify/MenuLayer.hpp>

using namespace geode::prelude;

bool fromMyMod = false;
bool saving = false;
bool shouldSync = true;
Notification* notification;

class $modify(MenuLayer) {
    void FLAlert_Clicked(FLAlertLayer* alert, bool b) {
        if(b && alert->getTag() == 0 && GJAccountManager::sharedState()->m_accountID != 0){
            fromMyMod = true;
            saving = true;


            notification = Notification::create("Saving account... Press close button or ESC key to cancel.", NotificationIcon::Loading, 0);
            notification->show();

            GJAccountManager::sharedState()->getAccountBackupURL();
            
            return;
        }

        return MenuLayer::FLAlert_Clicked(alert, b);
    }

    bool init() {
        auto ret = MenuLayer::init();

        if(shouldSync) {
            shouldSync = false;
            fromMyMod = true;

            notification = Notification::create("Syncing account...", NotificationIcon::Loading, 0);
            notification->show();
            
            GJAccountManager::sharedState()->getAccountSyncURL();
        }

        return ret;
    }

    void onQuit(CCObject* sender) {
        if(saving) {
            this->endGame();
        }

        MenuLayer::onQuit(sender);
    }
};

class $modify(GJAccountManager) {
    void onProcessHttpRequestCompleted(cocos2d::extension::CCHttpClient* client, cocos2d::extension::CCHttpResponse* response) {
        if(fromMyMod && this->m_activeDownloads->objectForKey("bak_account")){
            this->m_activeDownloads->removeObjectForKey("bak_account");

            notification->setIcon(response->isSucceed() ? NotificationIcon::Success : NotificationIcon::Error);
            notification->setString(response->isSucceed() ? "Backup successful." : "Backup failed!");
            notification->setTime(1.0f);
            
            fromMyMod = false;
            saving = false;

            CCDirector::sharedDirector()->getActionManager()->addAction(
                CCSequence::create(
                    CCDelayTime::create(0.25f),
                    CCCallFunc::create(CCDirector::sharedDirector()->getRunningScene(), SEL_CallFunc(&MenuLayer::endGame)),
                    nullptr
                ), CCDirector::sharedDirector()->getRunningScene(), false
            );

            return;
        }
        
        if(fromMyMod && this->m_activeDownloads->objectForKey("sync_account")) {
            this->m_activeDownloads->removeObjectForKey("sync_account");

            notification->setIcon(response->isSucceed() ? NotificationIcon::Success : NotificationIcon::Error);
            notification->setString(response->isSucceed() ? "Saved data has been downloaded." : "Sync failed!");
            notification->setTime(1.0f);

            fromMyMod = false;
            return;
        }

        return GJAccountManager::onProcessHttpRequestCompleted(client, response);
    }
};