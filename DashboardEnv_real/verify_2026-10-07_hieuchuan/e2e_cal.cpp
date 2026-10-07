// E2E: hiệu chuẩn phổ RF nhiều tần số. Backend THẬT <-> pty <-> fakerf.py
// usage: e2e_cal <tty-link> <ctrl-file> <config.ini> <config2.ini>
#include <QGuiApplication>
#include <QTimer>
#include <QFile>
#include <QSettings>
#include <QTextStream>
#include <QFileInfo>
#include <cstdio>
#include <cmath>
#include <functional>
#include "dashboardmanager.h"
#include "rfcontroller.h"

static int fails = 0, passes = 0;
#define CHECK(c, m) do { if (c) { ++passes; std::printf("PASS  %s\n", qPrintable(QString(m))); } else { ++fails; std::printf("FAIL  %s\n", qPrintable(QString(m))); } std::fflush(stdout); } while (0)
struct Steps { QList<QPair<int, std::function<void()>>> l; void add(int ms, std::function<void()> f) { l.append({ms, f}); }
  void run(int i = 0) { if (i >= l.size()) return; QTimer::singleShot(l[i].first, [this, i] { l[i].second(); run(i + 1); }); } };

static QString g_ctrl;
static void tone(const QString &t, double lvl, int gain = 0, double lo = 2450) {
    QFile f(g_ctrl); f.open(QIODevice::WriteOnly | QIODevice::Truncate);
    QTextStream(&f) << t << " " << lvl << " " << gain << " " << lo << "\n";
}
static QVariantMap point(RfController *rf, double mhz) {
    for (const QVariant &v : rf->calPoints()) { QVariantMap m = v.toMap(); if (std::abs(m["mhz"].toDouble() - mhz) < 0.01) return m; }
    return {};
}
static bool near(double a, double b, double tol = 0.1) { return std::abs(a - b) <= tol; }

int main(int argc, char **argv)
{
    g_ctrl = argv[2];
    qputenv("DASHBOARDENV_CONFIG", argv[3]);
    QFile::remove(argv[3]);
    QGuiApplication app(argc, argv);
    tone("none", -40);
    DashboardManager dm;
    RfController *rf = dm.rfController();
    auto logHas = [&](const QString &x) { for (auto &l : dm.commandLog()) if (l.contains(x)) return true; return false; };
    Steps S;

    S.add(100, [&] {
        CHECK(rf->calPoints().size() == 5 && !rf->calibrated(), "5 điểm mặc định 2440..2460, chưa hiệu chuẩn");
        QSettings ini(argv[3], QSettings::IniFormat);
        CHECK(ini.value("rf/calPointsMHz").toString() == "2440,2445,2450,2455,2460", "ghi rf/calPointsMHz mặc định vào config.ini");
        dm.setComPort(QFileInfo(argv[1]).symLinkTarget()); dm.setBaudRate(115200);
        dm.startSession();
    });
    S.add(2000, [&] {
        CHECK(rf->waveSource() == "live" && rf->rfStatusValid(), "nhận phổ thật + RFST");
        CHECK(!dm.calibrateRf(-10), "không có tín hiệu -> từ chối hiệu chuẩn");
        tone("2445", -40);
    });
    S.add(600, [&] {
        CHECK(dm.calibrateRf(-10), "phát 2445 MHz -40 dBFS, chuẩn -10 dBm -> hiệu chuẩn OK");
        QVariantMap p = point(rf, 2445);
        CHECK(p["valid"].toBool() && near(p["offsetDb"].toDouble(), 30.0), QString("điểm 2445 hệ số = %1 (mong 30.0)").arg(p["offsetDb"].toDouble()));
        CHECK(!point(rf, 2450)["valid"].toBool(), "không đụng điểm khác");
        tone("2455", -42);
    });
    S.add(600, [&] {
        CHECK(dm.calibrateRf(-10), "phát 2455 MHz -42 dBFS -> hiệu chuẩn OK");
        CHECK(near(point(rf, 2455)["offsetDb"].toDouble(), 32.0), QString("điểm 2455 hệ số = %1 (mong 32.0)").arg(point(rf, 2455)["offsetDb"].toDouble()));
        tone("2450", -41);
    });
    S.add(600, [&] {
        CHECK(near(rf->calOffsetDb(), 31.0), QString("2450 (giữa 2445/2455): nội suy = %1 (mong 31.0)").arg(rf->calOffsetDb()));
        CHECK(near(rf->totalLevel(), -10.0, 0.15), QString("tổng hiển thị = %1 dBm (mong -10)").arg(rf->totalLevel()));
        CHECK(rf->calInfo().contains("nội suy 2445–2455"), "calInfo: " + rf->calInfo());
        CHECK(point(rf, 2445)["active"].toBool() && point(rf, 2455)["active"].toBool(), "2 ô 2445/2455 được tô 'đang dùng'");
        CHECK(rf->spectrumUnit() == "dBm", "đơn vị dBm");
        tone("2470", -40);
    });
    S.add(600, [&] {
        CHECK(!dm.calibrateRf(-10), "đỉnh 2470 MHz xa mọi điểm -> từ chối");
        tone("2465", -40);
    });
    S.add(600, [&] {
        CHECK(near(rf->calOffsetDb(), 32.0) && rf->calInfo().contains("theo điểm 2455"),
              QString("2465 ngoài dải -> dùng điểm biên 2455 (%1) | %2").arg(rf->calOffsetDb()).arg(rf->calInfo()));
        tone("none", -40);
    });
    S.add(800, [&] {
        CHECK(near(rf->calOffsetDb(), 32.0), QString("mất tín hiệu -> GIỮ hệ số cũ (%1), không nhảy theo nhiễu").arg(rf->calOffsetDb()));
        tone("2445", -40, 10);
    });
    S.add(1600, [&] {
        CHECK(rf->calMismatch() && point(rf, 2445)["mismatch"].toBool(), "gain đổi 0 -> 10 dB -> cảnh báo LO/gain khác");
        tone("2445", -40, 0);
        dm.executeCommand("cal");
        for (auto &l : dm.commandLog()) if (l.contains("MHz :") || l.contains("Đang dùng")) std::printf("      | %s\n", qPrintable(l));
        CHECK(logHas("2445.0 MHz :  29.99 dB") && logHas("2455.0 MHz :  31.99 dB") && logHas("2440.0 MHz : --"), "lệnh cal in bảng hệ số");
        // Lưu bền: 1 DashboardManager mới đọc lại đúng bảng.
        DashboardManager dm2;
        CHECK(near(point(dm2.rfController(), 2445)["offsetDb"].toDouble(), 30.0) &&
              near(point(dm2.rfController(), 2455)["offsetDb"].toDouble(), 32.0) &&
              !point(dm2.rfController(), 2460)["valid"].toBool(), "mở lại app: bảng hệ số nạp lại từ config.ini");
    });
    S.add(1600, [&] {
        CHECK(!rf->calMismatch(), "gain về 0 -> hết cảnh báo");
        dm.executeCommand("cal clear 2445");
        CHECK(!point(rf, 2445)["valid"].toBool() && point(rf, 2455)["valid"].toBool(), "cal clear 2445 chỉ xoá điểm 2445");
        dm.clearRfCalPoint(2455);
        CHECK(!rf->calibrated() && rf->spectrumUnit() == "dBFS", "xoá hết điểm -> về dBFS");
        dm.calibrateRf(-10);
        dm.executeCommand("cal clear all");
        CHECK(!rf->calibrated(), "cal clear all");
        QSettings ini(argv[3], QSettings::IniFormat);
        CHECK(!ini.contains("rf/cal/2445/offsetDb") && !ini.contains("rf/cal/2455/offsetDb"), "config.ini sạch các điểm đã xoá");
        dm.stopSession();
        // Chuyển đổi từ bản cũ 1 hệ số.
        QFile::remove(argv[4]);
        { QSettings old(argv[4], QSettings::IniFormat);
          old.setValue("rf/calOffsetDb", 26.5); old.setValue("rf/calLoHz", 2450000000ULL); old.setValue("rf/calGainDb", 0); old.sync(); }
        qputenv("DASHBOARDENV_CONFIG", argv[4]);
        DashboardManager dm3;
        QVariantMap p = point(dm3.rfController(), 2450);
        CHECK(p["valid"].toBool() && near(p["offsetDb"].toDouble(), 26.5), "hệ số cũ 26.5 dB -> chuyển sang điểm 2450");
        QSettings after(argv[4], QSettings::IniFormat);
        CHECK(!after.contains("rf/calOffsetDb") && after.contains("rf/cal/2450/offsetDb"), "khoá cũ đã gỡ, khoá mới đã ghi");
        std::printf("RESULT(cal): %d passed, %d failed\n", passes, fails);
        app.exit(fails ? 1 : 0);
    });
    S.run();
    return app.exec();
}
