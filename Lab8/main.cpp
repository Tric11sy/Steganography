#include <QtWidgets>
#include <QImage>
#include <QVector>
#include <QColor>
#include <QFileDialog>
#include <QComboBox>
#include <QLabel>
#include <QVBoxLayout>
#include <QPushButton>
#include <QDebug>
#include <cmath>
#include <random>
#include <complex>
#include <iostream>


// Enum для выбора метода встраивания
enum EmbeddingMethod {
    CaseManipulation,
    WhitespaceManipulation,
    ImageEmbedding,
    SynonymSubstitution
};

// Функция для преобразования QImage в binary string
std::string imageToBinaryString(const QImage& image) {
    QByteArray byteArray;
    QBuffer buffer(&byteArray);
    buffer.open(QIODevice::WriteOnly);
    image.save(&buffer, "PNG"); 
    QByteArray base64Data = byteArray.toBase64();
    return std::string(base64Data.constData(), base64Data.length());
}

// Функция для преобразования binary string в QImage
QImage binaryStringToImage(const std::string& binaryString) {
    QByteArray base64Data = QByteArray::fromStdString(binaryString);
    QByteArray imageData = QByteArray::fromBase64(base64Data);
    QImage image;
    image.loadFromData(imageData);
    return image;
}

std::string dataToBinaryString(const std::string& data) {
    std::string binaryData;
    for (char c : data) {
        for (int i = 7; i >= 0; --i) {
            binaryData += ((c >> i) & 1) ? '1' : '0';
        }
    }
    return binaryData;
}

std::string binaryStringToData(const std::string& binaryData) {
    std::string extractedData;
    for (size_t i = 0; i < binaryData.length(); i += 8) {
        if (i + 8 <= binaryData.length()) {
            std::string byte = binaryData.substr(i, 8);
            int charCode = std::stoi(byte, nullptr, 2);
            extractedData += static_cast<char>(charCode);
        } else {
            break;
        }
    }
    return extractedData;
}

// Функция для встраивания информации с использованием регистра букв
std::string embedCaseManipulation(const std::string& text, const std::string& data) {
    std::string binaryData = dataToBinaryString(data);
    std::string result = text;
    int dataIndex = 0;
    for (size_t i = 0; i < result.length() && dataIndex < binaryData.length(); ++i) {
        if (isalpha(result[i])) {
            if (binaryData[dataIndex] == '1') {
                result[i] = toupper(result[i]);
            } else {
                result[i] = tolower(result[i]);
            }
            dataIndex++;
        }
    }

    if (dataIndex < binaryData.length()) {
        qDebug() << "Warning: Not all data could be embedded using Case Manipulation.";
    }

    return result;
}

// Функция для извлечения информации с использованием регистра букв 
std::string extractCaseManipulation(const std::string& text) {
    std::string binaryData;
    for (char c : text) {
        if (isalpha(c)) {
            binaryData += isupper(c) ? '1' : '0';
        }
    }

    // Дополнительная логика для отсечения мусора: находим конец осмысленного сообщения
    std::string extractedData = binaryStringToData(binaryData);

    // Находим первый нулевой символ (конец строки)
    size_t endPos = extractedData.find('\0');

    // Если нулевой символ найден, обрезаем строку до него
    if (endPos != std::string::npos) {
        extractedData.erase(endPos);
    }

    return extractedData;
}

// Функция для встраивания информации с использованием пробелов
std::string embedWhitespaceManipulation(const std::string& text, const std::string& data) {
    std::string binaryData = dataToBinaryString(data);
    std::string result = text;
    int dataIndex = 0;
    size_t i = 0;
    while (i < result.length() && dataIndex < binaryData.length()) {
        if (result[i] == ' ') {
            if (binaryData[dataIndex] == '1') {
                result.insert(i, " ");
                i += 2;
            } else {
                i++;
            }
            dataIndex++;
        } else {
            i++;
        }
    }

    if (dataIndex < binaryData.length()) {
        qDebug() << "Warning: Not all data could be embedded using Whitespace Manipulation.";
    }

    return result;
}

// Функция для извлечения информации с использованием пробелов
std::string extractWhitespaceManipulation(const std::string& text) {
    std::string binaryData;
    for (size_t i = 0; i < text.length(); ++i) {
        if (text[i] == ' ') {
            if (i + 1 < text.length() && text[i + 1] == ' ') {
                binaryData += '1';
                i++;
            } else {
                binaryData += '0';
            }
        }
    }

    std::string extractedData = binaryStringToData(binaryData);
    std::string cleanedData;

    // Оставляем только допустимые ASCII символы (32-126)
    for (char c : extractedData) {
        if (c >= 32 && c <= 126) {
            cleanedData += c;
        } else {
            break;
        }
    }

    return cleanedData;
}

// Функция для встраивания изображения в текст
std::string embedImage(const std::string& text, const std::string& imageData) {
    return text + "\n\n----IMAGE_START----\n" + imageData + "\n----IMAGE_END----\n";
}

// Функция для извлечения изображения из текста
std::string extractImage(const std::string& text) {
    size_t startPos = text.find("----IMAGE_START----\n");
    if (startPos == std::string::npos) return "";

    startPos += strlen("----IMAGE_START----\n");
    size_t endPos = text.find("\n----IMAGE_END----\n", startPos);
    if (endPos == std::string::npos) return "";

    return text.substr(startPos, endPos - startPos);
}


QMap<QString, QSet<QString>> synonymDatabase = {
    {"happy", {"joyful", "glad", "content"}},
    {"sad", {"unhappy", "depressed", "sorrowful"}},
    {"big", {"large", "huge", "enormous"}},
    {"small", {"tiny", "little", "miniature"}},
    {"quick", {"fast", "swift"}},
    {"jumps", {"leaps", "hops"}},
    {"lazy", {"idle", "sluggish"}},
};

QString findSynonym(const QString& word, int bit) {
    if (!synonymDatabase.contains(word.toLower())) {
        return word;
    }

    QSet<QString> synonyms = synonymDatabase[word.toLower()];
    if (synonyms.isEmpty()) {
        return word;
    }

    int synonymIndex = 0;
    for (const QString& synonym : synonyms) {
        if ((synonymIndex % 2) == bit) {
            return synonym;
        }
        synonymIndex++;
    }

    return word;
}

// Функция для встраивания информации с использованием синонимов
std::string embedSynonymSubstitution(const std::string& text, const std::string& data) {
    std::string binaryData = dataToBinaryString(data);
    QString qText = QString::fromStdString(text);
    QStringList words = qText.split(' ', Qt::SkipEmptyParts);
    QStringList resultWords;
    int dataIndex = 0;

    for (int i = 0; i < words.size() && dataIndex < binaryData.length(); ++i) {
        QString originalWord = words[i];
        if (synonymDatabase.contains(originalWord.toLower())) {
            int bit = binaryData[dataIndex] - '0';
            QString synonym = findSynonym(originalWord, bit);
            resultWords.append(synonym);
            dataIndex++;
        } else {
            resultWords.append(originalWord);
        }
    }

    if (dataIndex < binaryData.length()) {
        qDebug() << "Warning: Not all data could be embedded using Synonym Substitution.";
    }

    return resultWords.join(" ").toStdString();
}

// Функция для извлечения информации с использованием синонимов
std::string extractSynonymSubstitution(const std::string& text) {
    QString qText = QString::fromStdString(text);
    QStringList words = qText.split(' ', Qt::SkipEmptyParts);
    std::string binaryData;

    for (const QString& word : words) {
        bool found = false;
        for (auto it = synonymDatabase.begin(); it != synonymDatabase.end(); ++it) {
            if (it.value().contains(word.toLower())) {
                int synonymIndex = 0;
                for (const QString& synonym : it.value()) {
                    if (synonym.toLower() == word.toLower()) {
                        binaryData += ((synonymIndex % 2) == 0) ? '0' : '1';
                        found = true;
                        break;
                    }
                    synonymIndex++;
                }
                if (found) break;
            }
        }
    }

    std::string extractedData = binaryStringToData(binaryData);
    size_t endPos = extractedData.find('\0');
    if (endPos != std::string::npos) {
        extractedData.erase(endPos);
    }
    return extractedData;
}

// Главная функция встраивания, выбирающая метод
std::string embedData(const std::string& text, const std::string& data, EmbeddingMethod method) {
    switch (method) {
        case CaseManipulation:
            return embedCaseManipulation(text, data);
        case WhitespaceManipulation:
            return embedWhitespaceManipulation(text, data);
        case ImageEmbedding:
            return embedImage(text, data);
        case SynonymSubstitution:
            return embedSynonymSubstitution(text, data);
        default:
            qDebug() << "Error: Invalid embedding method.";
            return text;
    }
}

// Главная функция извлечения, выбирающая метод
std::string extractData(const std::string& text, EmbeddingMethod method) {
    switch (method) {
        case CaseManipulation:
            return extractCaseManipulation(text);
        case WhitespaceManipulation:
            return extractWhitespaceManipulation(text);
         case ImageEmbedding:
            return extractImage(text);
        case SynonymSubstitution:
            return extractSynonymSubstitution(text);
        default:
            qDebug() << "Error: Invalid embedding method.";
            return "";
    }
}

class MainWindow : public QWidget {
public:
    MainWindow() : QWidget() {
        setWindowTitle("Text Steganography");

        // UI elements (from previous version)
        textLabel = new QLabel("Original Text:");
        textEdit = new QTextEdit();

        dataLabel = new QLabel("Data to Embed (or select image):");
        dataEdit = new QTextEdit();

        imageButton = new QPushButton("Select Image");

        embeddingMethodLabel = new QLabel("Embedding Method:");
        embeddingMethodComboBox = new QComboBox();
        embeddingMethodComboBox->addItem("Case Manipulation");
        embeddingMethodComboBox->addItem("Whitespace Manipulation");
        embeddingMethodComboBox->addItem("Image Embedding");
        embeddingMethodComboBox->addItem("Synonym Substitution");

        embeddedTextLabel = new QLabel("Embedded Text:");
        embeddedTextEdit = new QTextEdit();
        embeddedTextEdit->setReadOnly(true);

        extractedDataLabel = new QLabel("Extracted Data (or Image):");
        extractedDataEdit = new QTextEdit();
        extractedDataEdit->setReadOnly(true);
        imageDisplayLabel = new QLabel();
        imageDisplayLabel->setAlignment(Qt::AlignCenter);

        embedButton = new QPushButton("Embed Data");
        extractButton = new QPushButton("Extract Data");

        QVBoxLayout *mainLayout = new QVBoxLayout(this);
        mainLayout->addWidget(textLabel);
        mainLayout->addWidget(textEdit);
        mainLayout->addWidget(dataLabel);

        QHBoxLayout *dataLayout = new QHBoxLayout();
        dataLayout->addWidget(dataEdit);
        dataLayout->addWidget(imageButton);
        mainLayout->addLayout(dataLayout);

        mainLayout->addWidget(embeddingMethodLabel);
        mainLayout->addWidget(embeddingMethodComboBox);
        mainLayout->addWidget(embeddedTextLabel);
        mainLayout->addWidget(embeddedTextEdit);
        mainLayout->addWidget(extractedDataLabel);
        mainLayout->addWidget(extractedDataEdit);
        mainLayout->addWidget(imageDisplayLabel);

        QHBoxLayout *buttonLayout = new QHBoxLayout();
        buttonLayout->addWidget(embedButton);
        buttonLayout->addWidget(extractButton);
        mainLayout->addLayout(buttonLayout);

        connect(embedButton, &QPushButton::clicked, this, &MainWindow::embedButtonClicked);
        connect(extractButton, &QPushButton::clicked, this, &MainWindow::extractButtonClicked);
        connect(imageButton, &QPushButton::clicked, this, &MainWindow::imageButtonClicked);
    }

private slots:
    void embedButtonClicked() {
        std::string text = textEdit->toPlainText().toStdString();
        std::string data = dataEdit->toPlainText().toStdString();
        EmbeddingMethod method = static_cast<EmbeddingMethod>(embeddingMethodComboBox->currentIndex());

        std::string embedded = embedData(text, data, method);
        embeddedTextEdit->setPlainText(QString::fromStdString(embedded));
        extractedDataEdit->clear();
        imageDisplayLabel->clear();
    }

    void extractButtonClicked() {
        std::string text = embeddedTextEdit->toPlainText().toStdString();
        EmbeddingMethod method = static_cast<EmbeddingMethod>(embeddingMethodComboBox->currentIndex());

        std::string extracted = extractData(text, method);
        extractedDataEdit->setPlainText(QString::fromStdString(extracted));
        imageDisplayLabel->clear();

        if (method == ImageEmbedding) {
            QImage image = binaryStringToImage(extracted);
            if (!image.isNull()) {
                QPixmap pixmap = QPixmap::fromImage(image);
                imageDisplayLabel->setPixmap(pixmap.scaled(imageDisplayLabel->size(), Qt::KeepAspectRatio));
            } else {
                QMessageBox::warning(this, "Error", "Could not decode image from extracted data.");
            }
        }
    }

    void imageButtonClicked() {
        QString fileName = QFileDialog::getOpenFileName(this, "Select Image", "", "Images (*.png *.jpg *.jpeg *.bmp)");
        if (!fileName.isEmpty()) {
            QImage image(fileName);
            if (image.isNull()) {
                QMessageBox::warning(this, "Error", "Could not load image.");
                return;
            }
            std::string base64Image = imageToBinaryString(image);
            dataEdit->setPlainText(QString::fromStdString(base64Image));
        }
    }

private:
    QLabel *textLabel;
    QTextEdit *textEdit;

    QLabel *dataLabel;
    QTextEdit *dataEdit;
    QPushButton *imageButton;

    QLabel *embeddingMethodLabel;
    QComboBox *embeddingMethodComboBox;

    QLabel *embeddedTextLabel;
    QTextEdit *embeddedTextEdit;

    QLabel *extractedDataLabel;
    QTextEdit *extractedDataEdit;
    QLabel *imageDisplayLabel;

    QPushButton *embedButton;
    QPushButton *extractButton;
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    MainWindow window;
    window.show();

    return app.exec();
}