#include "../../emojicategory.h"
#include "../../emojidict.h"
#include <QCoreApplication>
#include <QDataStream>
#include <QDebug>
#include <QDomDocument>
#include <QFile>

class UnicodeEmojiParser
{
public:
    void loadEmojiTest(const QString &file)
    {
        QMap<QString, QString> descriptionMapping;
        QFile emojiFile(file);
        emojiFile.open(QIODevice::ReadOnly);
        QByteArray line;
        int currentGroup = 0;
        while (!emojiFile.atEnd()) {
            line = emojiFile.readLine().trimmed();
            if (line.startsWith("# group: ")) {
                currentGroup += 1;
                line = line.mid(9).replace("&", "&&");
                if (getCategoryNames().indexOf(line) != currentGroup - 1) {
                    qFatal("Please update emojicategory to match emoji-test.txt.");
                }
            }
            if (line.startsWith("#")) {
                continue;
            }
            // code points; status # emoji name
            auto segments = line.split(';');
            if (segments.size() != 2) {
                continue;
            }
            auto metadata = segments[1].split('#');
            if (metadata.size() != 2) {
                continue;
            }
            auto codes = segments[0].trimmed().split(' ');
            if (codes.isEmpty()) {
                continue;
            }
            bool fail = false;
            std::vector<quint32> ucs4;
            for (const auto &code : codes) {
                bool ok = false;
                auto codeInt = code.toInt(&ok, 16);
                if (!ok) {
                    fail = true;
                    break;
                }
                ucs4.push_back(codeInt);
            }
            if (fail) {
                continue;
            }
            const auto emoji = QString::fromUcs4(ucs4.data(), ucs4.size());
            const auto status = metadata[0].trimmed();
            const auto description = metadata[1].trimmed();
            if (status == "fully-qualified") {
                Emoji emojiData;
                emojiData.content = emoji;
                emojiData.category = currentGroup;
                m_emojiIndex[emoji] = m_emojis.size();
                m_emojis.append(emojiData);

                descriptionMapping[description] = emoji;
            } else {
                auto fullyQualified = descriptionMapping.value(description, QString());
                if (!fullyQualified.isEmpty()) {
                    m_variantMapping[emoji] = fullyQualified;
                }
            }
        }
    }

    void loadCldrAnnotation(const QString &file)
    {
        QFile cldrFile(file);
        cldrFile.open(QIODevice::ReadOnly);
        QDomDocument cldr;
        cldr.setContent(cldrFile.readAll());

        auto annotations = cldr.elementsByTagName("annotation");
        for (int i = 0; i < annotations.count(); i++) {
            auto attributes = annotations.at(i).attributes();
            if (!attributes.contains("cp")) {
                continue;
            }
            auto emoji = attributes.namedItem("cp").nodeValue();
            if (!m_emojiIndex.contains(emoji)) {
                emoji = m_variantMapping.value(emoji, QString());
            }
            if (emoji.isEmpty()) {
                continue;
            }
            if (attributes.contains("type") && attributes.namedItem("type").nodeValue() == "tts") {
                m_emojis[m_emojiIndex[emoji]].description = annotations.at(i).firstChild().toText().data();
            } else {
                m_emojis[m_emojiIndex[emoji]].annotations = annotations.at(i).firstChild().toText().data().split(" | ");
            }
        }
    }

    const QList<Emoji> &emojis() const
    {
        return m_emojis;
    }

private:
    // We want to preserve the order of data in emoji-test.txt.
    QList<Emoji> m_emojis;
    QMap<QString, int> m_emojiIndex;
    QMap<QString, QString> m_variantMapping;
};

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    ;

    UnicodeEmojiParser parser;
    parser.loadEmojiTest(argv[1]);
    parser.loadCldrAnnotation(argv[2]);
    parser.loadCldrAnnotation(argv[3]);

    QFile output(argv[4]);
    output.open(QIODevice::WriteOnly);
    QDataStream stream(&output);
    // Endianess is enforced here to ensure it's not relevant to architecture.
    stream.setByteOrder(QDataStream::LittleEndian);
    stream << parser.emojis();

    output.close();

    return 0;
}
