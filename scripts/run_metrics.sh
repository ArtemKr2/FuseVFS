#!/usr/bin/env bash
set -e
OUT=reports
SRC1=src
SRC2=Tests

mkdir -p $OUT

lizard  $SRC1 $SRC2 --csv  > $OUT/lizard.csv
lizard $SRC1 $SRC2 --html -o $OUT/lizard.html

cloc    $SRC1 $SRC2 --by-file --csv --out=$OUT/cloc.csv

metrix++ reset   --db $OUT/.mpp.db
metrix++ collect --std.auto --db $OUT/.mpp.db  $SRC1 $SRC2
metrix++ view    --db $OUT/.mpp.db --format csv > $OUT/metrix.csv
