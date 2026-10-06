#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "backend/dashboardmanager.h"
#include <QFont>
#include <QGuiApplication>
#include <QQuickStyle>
#include <QFontDatabase>
#include <QSettings>

int main(int argc, char *argv[]) {
    // 1. Khởi tạo ứng dụng với cấu hình đầy đủ của Widgets (bắt buộc cho QtCharts)
    QApplication app(argc, argv);

    // Ép mọi QSettings mặc định (kể cả QSettings settings; không tham số
    // trong DashboardManager) ra file *.ini thay vì registry Windows/plist
    // macOS - đúng yêu cầu "lưu config file *.ini". Phải gọi TRƯỚC khi bất kỳ
    // QSettings nào được tạo (DashboardManager tạo 1 cái trong constructor
    // ngay bên dưới) - và sau khi setOrganizationName/setApplicationName vì
    // QSettings mặc định dùng 2 tên đó để đặt tên file .ini.
    app.setOrganizationName("VASC");
    app.setApplicationName("AvionicsDashboard");
    QSettings::setDefaultFormat(QSettings::IniFormat);

    QQuickStyle ::setStyle("Basic");
    QFontDatabase::addApplicationFont(":/qt/qml/DashboardEnv/assets/typography/FSMagistral-Light.ttf");
    QFontDatabase::addApplicationFont(":/qt/qml/DashboardEnv/assets/typography/FSMagistral-Bold.ttf");
    QFontDatabase::addApplicationFont(":/qt/qml/DashboardEnv/assets/typography/FSMagistral-Medium.ttf");
    QFontDatabase::addApplicationFont(":/qt/qml/DashboardEnv/assets/typography/FSMagistral-ExtraBold.ttf");
    QFontDatabase::addApplicationFont(":/qt/qml/DashboardEnv/assets/typography/FSMagistral-Book.ttf");
    QFontDatabase::addApplicationFont(":/qt/qml/DashboardEnv/assets/typography/7seg.ttf");

    QFont globalFont("FS Magistral",8);
    app.setFont(globalFont);

    QFont segFont("Seven Segment",8);

    // 3. Khởi tạo "Trái tim" Back-end điều khiển hệ thống
    DashboardManager dbManager;

    // 4. Khởi tạo QML Engine để load giao diện
    QQmlApplicationEngine engine;

    engine.rootContext()->setContextProperty("dashboard", &dbManager);

    // 6. Định nghĩa đường dẫn nạp file giao diện chính (Đã map theo CMake QML Module)
    const QUrl url(QStringLiteral("DashboardEnv/qml/Main.qml"));

    // Kết nối signal bảo vệ: Nếu load file QML thất bại thì tắt app ngay lập tức để tránh chạy ngầm lỗi
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
                         if (!obj && url == objUrl)
                             QCoreApplication::exit(-1);
                     }, Qt::QueuedConnection);

    // 7. Kích hoạt nạp giao diện
    engine.load(url);

    // 8. Chạy vòng lặp sự kiện (Event Loop) của Qt
    return app.exec();
}