#!/usr/bin/env bash

# A naive check to ensure it is run under the correct working directory.
if [ ! -d dictbuilder ]; then
  echo "Please run update.sh under its own directory."
  exit 1
fi

pushd .
rm -rf emojidict
mkdir emojidict
cd emojidict

# Make sure we have good font support before update emoji version.
EMOJI_VERSION=14.0
wget http://www.unicode.org/Public/emoji/$EMOJI_VERSION/emoji-test.txt

echo '// Generated from emoji-test.txt' > ../../emojicategory.cpp
echo '#include "emojicategory.h"' >> ../../emojicategory.cpp
echo 'const QStringList &getCategoryNames()' >> ../../emojicategory.cpp
echo '{' >> ../../emojicategory.cpp
echo '    static const QStringList names = {' >> ../../emojicategory.cpp
# Generate category name, also make sure we escape qt accelerator
grep "# group" emoji-test.txt  | sed 's|# group: \(.*\)|        I18N_NOOP2("Emoji Category", "\1"),|' | sed 's|&|&&|g' >> ../../emojicategory.cpp
echo '    };' >> ../../emojicategory.cpp
echo '    return names;' >> ../../emojicategory.cpp
echo '}' >> ../../emojicategory.cpp

CLDR_VERSION=41.0
CLDR_FILE=cldr-common-$CLDR_VERSION.zip
wget https://unicode.org/Public/cldr/${CLDR_VERSION%.*}/$CLDR_FILE
unzip $CLDR_FILE

mkdir builder
cd builder
cmake ../../dictbuilder
cmake --build .
rm ../../*.dict
for f in ../common/annotations/*.xml; do
  ANNOTATION_FILE=`basename $f`
  if [ -f ../common/annotationsDerived/$ANNOTATION_FILE ]; then
    ./dictbuilder ../emoji-test.txt $f ../common/annotationsDerived/$ANNOTATION_FILE ../../${ANNOTATION_FILE/xml/dict}
  fi
done

popd
rm -rf emojidict

echo "Please remember to update the emoji category string in qml if there is new category name."
