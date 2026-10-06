cmake_minimum_required(VERSION 3.16)

project(DashboardEnv VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(QT_QML_GENERATE_QMLLS_INI ON)

find_package(Qt6 REQUIRED COMPONENTS Quick)
find_package(Qt6 REQUIRED COMPONENTS Core Qml Widgets SerialPort Gui Charts QuickControls2)
find_package(Qt6Widgets CONFIG)


qt_standard_project_setup(REQUIRES 6.8)

qt_add_executable(appDashboardEnv
    main.cpp
)
set_source_files_properties(qml/components/AppTheme.qml
    PROPERTIES QT_QML_SINGLETON_TYPE TRUE
)

qt_add_qml_module(appDashboardEnv
    URI DashboardEnv
    VERSION 1.0
    QML_FILES
        qml/Main.qml
        qml/components/BoxWrap.qml
        qml/views/ConnectionPanel.qml
        qml/views/PowerControlPanel.qml
        qml/views/TimeDelay.qml
        qml/components/MiniMonitorChart.qml
        qml/views/EnvelopeChartPanel.qml
        qml/views/CommandLogPanel.qml
        qml/views/Notification.qml
        qml/views/Detect.qml
        qml/views/Attenuation.qml
        qml/components/CustomComboBox.qml
        qml/components/CustomSwitch.qml
        qml/components/CustomBoxFiled.qml
        qml/components/ClampedTextField.qml
        qml/components/AppTheme.qml
    SOURCES
        backend/basepowersource.h backend/basepowersource.cpp
        backend/acsource.h backend/acsource.cpp
        backend/dcsource.h backend/dcsource.cpp
        backend/rfcontroller.h backend/rfcontroller.cpp
        backend/dashboardmanager.h backend/dashboardmanager.cpp
        backend/dashboardcli.cpp
        backend/mavlink_helper.h

    RESOURCES
        assets/typography/FSMagistral-Medium.ttf
        assets/typography/FSMagistral-Bold.ttf
        assets/typography/FSMagistral-Light.ttf
        assets/typography/FSMagistral-ExtraBold.ttf
        assets/typography/FSMagistral-Book.ttf
        assets/typography/7seg.ttf
        assets/images/ACicon.png
        assets/images/ATTIcon.png
        assets/images/Connected.png
        assets/images/DCicon.png
        assets/images/DelayIcon.png
        assets/images/SendIcon.png
        assets/images/notice.png
        assets/images/language.png
        assets/images/Viettel_Aerospace_Institute_logo.png
        assets/images/Moon.png
        assets/images/Sun.png
        assets/images/refresh.png
        assets/images/freq.png
        SOURCES backend/kria_helper.h
        SOURCES backend/vip_helper.h
        SOURCES backend/vip_helper.cpp
        QML_FILES
        QML_FILES qml/views/chartView.qml
)


# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(appDashboardEnv PROPERTIES
#    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.appDashboardEnv
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)


target_link_libraries(appDashboardEnv
    PRIVATE Qt6::Quick
    PRIVATE Qt6::SerialPort
    Qt6::Gui
    Qt6::Qml
    Qt6::Charts
    Qt6::Widgets
    Qt6::QuickControls2
)


install(TARGETS appDashboardEnv
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
