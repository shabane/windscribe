#include "textpixmap.h"
#include "utils/ws_assert.h"

namespace gui_locations {

static QString sanitizeForTextPixmap(const QString &text)
{
    QString res;
    auto u32Str = text.toUcs4();
    for (char32_t code : u32Str)
    {
        // Skip emojis, regional indicators, or variation selectors that trigger macOS CopyEmojiImage
        if (code >= 0x1F000 || (code >= 0x2600 && code <= 0x27BF) || (code >= 0xFE00 && code <= 0xFE0F))
        {
            continue;
        }
        res += QString::fromUcs4(&code, 1);
    }
    return res;
}

TextPixmap::TextPixmap(const QString &text, const QFont &font, qreal devicePixelRatio) : text_(text)
{
    QString safeText = sanitizeForTextPixmap(text);
    if (!safeText.isEmpty())
    {
        QFontMetrics fm(font);
        QRect rcText = fm.boundingRect(safeText);
        QPixmap pixmap(fm.horizontalAdvance(safeText)*devicePixelRatio, rcText.height()*devicePixelRatio);
        pixmap.setDevicePixelRatio(devicePixelRatio);
        pixmap.fill(Qt::transparent);
        {
            QPainter painter(&pixmap);
            painter.setPen(Qt::white);      // for current needs, we use only white color
            painter.setFont(font);
            painter.drawText(QRect(0, 0, fm.horizontalAdvance(safeText), rcText.height()), Qt::AlignLeft, safeText);
        }
        pixmap_ = IndependentPixmap(pixmap);
    }
}

void TextPixmaps::add(int id, const QString &text, const QFont &font, qreal devicePixelRatio)
{
    WS_ASSERT(!pixmaps_.contains(id));
    pixmaps_[id] = TextPixmap(text, font, devicePixelRatio);
}

void TextPixmaps::updateIfTextChanged(int id, const QString &text, const QFont &font, qreal devicePixelRatio)
{
    WS_ASSERT(pixmaps_.contains(id));
    auto it = pixmaps_.find(id);
    if (it != pixmaps_.end())
    {
        if (it.value().text() != text)
        {
            pixmaps_[id] = TextPixmap(text, font, devicePixelRatio);
        }
    }
}

IndependentPixmap TextPixmaps::pixmap(int id) const
{
    WS_ASSERT(pixmaps_.contains(id));
    return pixmaps_[id].pixmap();
}

} // namespace gui_locations
