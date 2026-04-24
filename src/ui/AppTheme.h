#ifndef APPTHEME_H
#define APPTHEME_H

#include <QString>
#include <QWidget>

namespace AppTheme {
    constexpr const char* Background = "#090B0E";
    constexpr const char* Workspace = "#0A0D11";
    constexpr const char* Surface = "#101419";
    constexpr const char* SurfaceAlt = "#151A20";
    constexpr const char* Hover = "#1B2128";
    constexpr const char* Border = "#262D36";
    constexpr const char* StrongBorder = "#343C47";
    constexpr const char* Accent = "#E5E7EB";
    constexpr const char* AccentPressed = "#C8CDD3";
    constexpr const char* Text = "#F2F4F7";
    constexpr const char* SecondaryText = "#A8B0BA";
    constexpr const char* MutedText = "#6F7884";
    constexpr const char* Danger = "#F2A0A0";
    constexpr const char* DangerBorder = "#6F3A3A";
    constexpr const char* DangerHover = "#231314";
    constexpr int Radius = 5;
    constexpr int Spacing = 10;

    QString styleSheet();
    QString dialogStyleSheet();

    void showInfoDialog(QWidget* parent, const QString& title, const QString& message);
    void showWarningDialog(QWidget* parent, const QString& title, const QString& message);
    void showErrorDialog(QWidget* parent, const QString& title, const QString& message);
    bool showConfirmDialog(QWidget* parent,
                           const QString& title,
                           const QString& message,
                           const QString& confirmText = "Confirm",
                           const QString& cancelText = "Cancel",
                           bool danger = false);
}

#endif 
