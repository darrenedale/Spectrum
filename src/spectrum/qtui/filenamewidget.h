//
// Created by darren on 12/10/2026.
//

#ifndef SPECTRUM_QTUI_FILENAMEWIDGET_H
#define SPECTRUM_QTUI_FILENAMEWIDGET_H

#include <QWidget>

class QLineEdit;
class QToolButton;

namespace Spectrum::QtUi
{
    class FilenameWidget
    : public QWidget
    {
        Q_OBJECT

        public:
            enum class Type
            {
                Open,
                Save,
            };

            explicit FilenameWidget(Type, QWidget * = nullptr) noexcept;
            explicit FilenameWidget(Type, const QString &, QWidget * = nullptr) noexcept;
            FilenameWidget(const FilenameWidget &) = delete;
            FilenameWidget(FilenameWidget &&) = delete;
            FilenameWidget operator=(const FilenameWidget &) = delete;
            FilenameWidget operator=(FilenameWidget &&) = delete;
            ~FilenameWidget() noexcept override;

            [[nodiscard]]
            Type type() const noexcept
            {
                return m_type;
            }

            void setType(Type) noexcept;

            [[nodiscard]]
            bool hasFilename() const noexcept;

            void setFilename(const QString &) noexcept;

            [[nodiscard]]
            QString filename() const noexcept;

            void setFilter(const QString &) noexcept;

        Q_SIGNALS:
            void filenameChanged(const QString &) const;
            void chooseFileActivated();

        private:
            void layoutWidget() noexcept;
            void connectWidgets() noexcept;

            Type m_type;
            std::unique_ptr<QLineEdit> m_filename;
            std::unique_ptr<QToolButton> m_choose;
            QString m_filter;
    };
} // Spectrum::QtUi

#endif //SPECTRUM_QTUI_FILENAMEWIDGET_H
