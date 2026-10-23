// SPDX-License-Identifier: GPL-2.0-or-later

#include "devicemodel.h"
#include "gamecontrollermoduledata.h"

#include <QAbstractItemModelTester>
#include <QSignalSpy>
#include <QTest>
#include <SDL2/SDL.h>

#include <memory>

// Keep SDL's real virtual joystick implementation, but let the DeviceModel
// treat those fixtures as physical devices. No real controller is required.
static bool s_emulatePhysical = true;
extern "C" SDL_bool __real_SDL_JoystickIsVirtual(int index);
extern "C" SDL_bool __wrap_SDL_JoystickIsVirtual(int index)
{
    return s_emulatePhysical ? SDL_FALSE : __real_SDL_JoystickIsVirtual(index);
}
extern "C" const char *__real_SDL_JoystickPath(SDL_Joystick *joystick);
extern "C" const char *__wrap_SDL_JoystickPath(SDL_Joystick *joystick)
{
    const auto path = __real_SDL_JoystickPath(joystick);
    return path ? path : "/dev/input/sonic-test-controller";
}

class GameControllerDiscoveryTest : public QObject {
    Q_OBJECT
private:
    QList<SDL_JoystickID> m_fixtures;

    static int indexFor(SDL_JoystickID id)
    {
        for (int index = 0; index < SDL_NumJoysticks(); ++index) {
            if (SDL_JoystickGetDeviceInstanceID(index) == id) {
                return index;
            }
        }
        return -1;
    }

    SDL_JoystickID attach()
    {
        SDL_VirtualJoystickDesc description{};
        description.version = SDL_VIRTUAL_JOYSTICK_DESC_VERSION;
        description.type = SDL_JOYSTICK_TYPE_GAMECONTROLLER;
        description.naxes = SDL_CONTROLLER_AXIS_MAX;
        description.nbuttons = SDL_CONTROLLER_BUTTON_MAX;
        description.nhats = 1;
        description.vendor_id = 0x1209;
        description.product_id = 0xbeef;
        description.name = "SonicDE discovery regression controller";
        const int index = SDL_JoystickAttachVirtualEx(&description);
        if (index < 0) {
            return -1;
        }
        const auto id = SDL_JoystickGetDeviceInstanceID(index);
        m_fixtures.append(id);
        return id;
    }

    static void drainEvents()
    {
        SDL_PumpEvents();
        SDL_FlushEvents(SDL_FIRSTEVENT, SDL_LASTEVENT);
    }

    static void poll(DeviceModel &model)
    {
        QVERIFY(QMetaObject::invokeMethod(&model, "poll", Qt::DirectConnection));
    }

private Q_SLOTS:
    void initTestCase()
    {
        QCOMPARE(SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER), 0);
    }

    void cleanup()
    {
        for (auto id : std::as_const(m_fixtures)) {
            const int index = indexFor(id);
            if (index >= 0) {
                QCOMPARE(SDL_JoystickDetachVirtual(index), 0);
            }
        }
        m_fixtures.clear();
        s_emulatePhysical = true;
        drainEvents();
    }

    void cleanupTestCase()
    {
        SDL_QuitSubSystem(SDL_INIT_GAMECONTROLLER);
    }

    void alreadyConnectedWithoutAddEvents()
    {
        const auto id = attach();
        QVERIFY2(id >= 0, SDL_GetError());
        drainEvents(); // Another SDL consumer has already taken the add event.
        DeviceModel model;
        QAbstractItemModelTester tester(&model, QAbstractItemModelTester::FailureReportingMode::Fatal);
        poll(model);
        QVERIFY(model.device(id));
    }

    void twoModelsSeeHotplugAndRemoval()
    {
        DeviceModel first;
        DeviceModel second;
        const auto id = attach();
        QVERIFY2(id >= 0, SDL_GetError());
        poll(first);
        poll(second);
        QVERIFY(first.device(id));
        QVERIFY(second.device(id));
        QCOMPARE(SDL_JoystickDetachVirtual(indexFor(id)), 0);
        poll(first);
        poll(second);
        QVERIFY(!first.device(id));
        QVERIFY(!second.device(id));
    }

    void moduleRelevanceAfterPageConsumesEvents()
    {
        DeviceModel page;
        const auto id = attach();
        QVERIFY2(id >= 0, SDL_GetError());
        poll(page);
        QVERIFY(page.device(id));
        GameControllerModuleData moduleData(nullptr);
        auto relevanceModel = moduleData.findChild<DeviceModel *>();
        QVERIFY(relevanceModel);
        poll(*relevanceModel);
        QVERIFY(relevanceModel->device(id));
        QVERIFY(moduleData.isRelevant());
    }

    void pageAfterModuleRelevanceConsumesEvents()
    {
        const auto id = attach();
        QVERIFY2(id >= 0, SDL_GetError());
        GameControllerModuleData moduleData(nullptr);
        auto relevanceModel = moduleData.findChild<DeviceModel *>();
        QVERIFY(relevanceModel);
        poll(*relevanceModel);
        QVERIFY(moduleData.isRelevant());
        QVERIFY(relevanceModel->device(id));
        DeviceModel page;
        poll(page);
        QVERIFY(page.device(id)); // visible navigation entry, but an empty page before the fix
    }

    void removalWithoutQueuedEvent()
    {
        const auto id = attach();
        QVERIFY2(id >= 0, SDL_GetError());
        DeviceModel model;
        poll(model);
        QVERIFY(model.device(id));
        QCOMPARE(SDL_JoystickDetachVirtual(indexFor(id)), 0);
        drainEvents();
        poll(model);
        QVERIFY(!model.device(id));
    }

    void staleAddIndicesDoNotMisidentifyDevices()
    {
        const auto removed = attach();
        const auto survivor = attach();
        QVERIFY(removed >= 0);
        QVERIFY(survivor >= 0);
        QCOMPARE(SDL_JoystickDetachVirtual(indexFor(removed)), 0);
        const auto replacement = attach();
        QVERIFY(replacement >= 0);
        DeviceModel model;
        QAbstractItemModelTester tester(&model, QAbstractItemModelTester::FailureReportingMode::Fatal);
        poll(model);
        QVERIFY(!model.device(removed));
        QVERIFY(model.device(survivor));
        QVERIFY(model.device(replacement));
    }

    void allModelsReceiveInput()
    {
        const auto id = attach();
        QVERIFY2(id >= 0, SDL_GetError());
        DeviceModel first;
        poll(first);
        DeviceModel second;
        poll(second);
        QVERIFY(first.device(id));
        QVERIFY(second.device(id));
        QSignalSpy firstAxis(first.device(id), &Device::leftAxisChanged);
        QSignalSpy secondAxis(second.device(id), &Device::leftAxisChanged);
        SDL_Event event{};
        if (first.device(id)->isController()) {
            event.type = SDL_CONTROLLERAXISMOTION;
            event.caxis.which = id;
            event.caxis.axis = SDL_CONTROLLER_AXIS_LEFTX;
            event.caxis.value = 16384;
        } else {
            event.type = SDL_JOYAXISMOTION;
            event.jaxis.which = id;
            event.jaxis.axis = 0;
            event.jaxis.value = 16384;
        }
        QCOMPARE(SDL_PushEvent(&event), 1);
        poll(first);
        poll(second);
        QVERIFY(!firstAxis.isEmpty());
        QVERIFY(!secondAxis.isEmpty());
        QCOMPARE(first.device(id)->leftAxisValue(), second.device(id)->leftAxisValue());
    }

    void closingOneModelKeepsOthersAlive()
    {
        const auto existing = attach();
        QVERIFY2(existing >= 0, SDL_GetError());
        auto first = std::make_unique<DeviceModel>();
        poll(*first);
        DeviceModel second;
        poll(second);
        QVERIFY(first->device(existing));
        QVERIFY(second.device(existing));
        first.reset();
        QVERIFY(SDL_WasInit(SDL_INIT_GAMECONTROLLER));
        QVERIFY(second.device(existing));
        const auto id = attach();
        QVERIFY2(id >= 0, SDL_GetError());
        drainEvents();
        poll(second);
        QVERIFY(second.device(id));
    }

    void virtualDevicesRemainExcluded()
    {
        s_emulatePhysical = false;
        const auto id = attach();
        QVERIFY2(id >= 0, SDL_GetError());
        DeviceModel model;
        poll(model);
        QVERIFY(!model.device(id));
    }
};

QTEST_GUILESS_MAIN(GameControllerDiscoveryTest)
#include "discoverytest.moc"
