static const char *type_errors[] = {
  "test/errors/type/binary_operators/__add.yasl",
  "test/errors/type/binary_operators/__bandnot.yasl",
  "test/errors/type/binary_operators/__band.yasl",
  "test/errors/type/binary_operators/__bor.yasl",
  "test/errors/type/binary_operators/__bshl.yasl",
  "test/errors/type/binary_operators/__bshr.yasl",
  "test/errors/type/binary_operators/__bxor.yasl",
  "test/errors/type/binary_operators/__exp.yasl",
  "test/errors/type/binary_operators/__fdiv.yasl",
  "test/errors/type/binary_operators/__ge.yasl",
  "test/errors/type/binary_operators/__gt.yasl",
  "test/errors/type/binary_operators/__idiv.yasl",
  "test/errors/type/binary_operators/__le.yasl",
  "test/errors/type/binary_operators/__lt.yasl",
  "test/errors/type/binary_operators/__mul.yasl",
  "test/errors/type/binary_operators/__sub.yasl",
  "test/errors/type/bool/tobool.yasl",
  "test/errors/type/bool/tostr.yasl",
  "test/errors/type/callable.yasl",
  "test/errors/type/collections/set/add2.yasl",
  "test/errors/type/collections/set/add3.yasl",
  "test/errors/type/collections/set/add.yasl",
  "test/errors/type/collections/set/__band2.yasl",
  "test/errors/type/collections/set/__band3.yasl",
  "test/errors/type/collections/set/__bandnot1.yasl",
  "test/errors/type/collections/set/__bandnot2.yasl",
  "test/errors/type/collections/set/__bandnot3.yasl",
  "test/errors/type/collections/set/__band.yasl",
  "test/errors/type/collections/set/__bor1.yasl",
  "test/errors/type/collections/set/__bor2.yasl",
  "test/errors/type/collections/set/__bor3.yasl",
  "test/errors/type/collections/set/__bxor1.yasl",
  "test/errors/type/collections/set/__bxor2.yasl",
  "test/errors/type/collections/set/__bxor3.yasl",
  "test/errors/type/collections/set/clear.yasl",
  "test/errors/type/collections/set/copy.yasl",
  "test/errors/type/collections/set/__eq2.yasl",
  "test/errors/type/collections/set/__eq3.yasl",
  "test/errors/type/collections/set/__eq.yasl",
  "test/errors/type/collections/set/__ge2.yasl",
  "test/errors/type/collections/set/__ge3.yasl",
  "test/errors/type/collections/set/__get.yasl",
  "test/errors/type/collections/set/__ge.yasl",
  "test/errors/type/collections/set/__gt2.yasl",
  "test/errors/type/collections/set/__gt3.yasl",
  "test/errors/type/collections/set/__gt.yasl",
  "test/errors/type/collections/set/__le2.yasl",
  "test/errors/type/collections/set/__le3.yasl",
  "test/errors/type/collections/set/__len.yasl",
  "test/errors/type/collections/set/__le.yasl",
  "test/errors/type/collections/set/__lt2.yasl",
  "test/errors/type/collections/set/__lt3.yasl",
  "test/errors/type/collections/set/__lt.yasl",
  "test/errors/type/collections/set/remove.yasl",
  "test/errors/type/collections/set/set.yasl",
  "test/errors/type/collections/set/tolist.yasl",
  "test/errors/type/collections/set/tostr.yasl",
  "test/errors/type/collections/table.yasl",
  "test/errors/type/float/tobool.yasl",
  "test/errors/type/float/tofloat.yasl",
  "test/errors/type/float/toint.yasl",
  "test/errors/type/float/tostr.yasl",
  "test/errors/type/int/tobool.yasl",
  "test/errors/type/int/tofloat.yasl",
  "test/errors/type/int/toint.yasl",
  "test/errors/type/int/tostr.yasl",
  "test/errors/type/io/flush.yasl",
  "test/errors/type/io/open1.yasl",
  "test/errors/type/io/open2.yasl",
  "test/errors/type/io/open3.yasl",
  "test/errors/type/io/open4.yasl",
  "test/errors/type/io/read1.yasl",
  "test/errors/type/io/read2.yasl",
  "test/errors/type/io/read3.yasl",
  "test/errors/type/io/read4.yasl",
  "test/errors/type/io/read5.yasl",
  "test/errors/type/io/write1.yasl",
  "test/errors/type/io/write2.yasl",
  "test/errors/type/io/write3.yasl",
  "test/errors/type/io/write4.yasl",
  "test/errors/type/list/__add1.yasl",
  "test/errors/type/list/__add2.yasl",
  "test/errors/type/list/__add3.yasl",
  "test/errors/type/list/__add4.yasl",
  "test/errors/type/list/clear.yasl",
  "test/errors/type/list/copy.yasl",
  "test/errors/type/list/join2.yasl",
  "test/errors/type/list/join3.yasl",
  "test/errors/type/list/join.yasl",
  "test/errors/type/list/__len.yasl",
  "test/errors/type/list/push.yasl",
  "test/errors/type/list/reverse.yasl",
  "test/errors/type/list/search.yasl",
  "test/errors/type/list/sort.yasl",
  "test/errors/type/list/tostr.yasl",
  "test/errors/type/math/abs.yasl",
  "test/errors/type/math/acos.yasl",
  "test/errors/type/math/asin.yasl",
  "test/errors/type/math/atan.yasl",
  "test/errors/type/math/ceil.yasl",
  "test/errors/type/math/cos.yasl",
  "test/errors/type/math/deg.yasl",
  "test/errors/type/math/exp.yasl",
  "test/errors/type/math/floor.yasl",
  "test/errors/type/math/gcd1.yasl",
  "test/errors/type/math/gcd2.yasl",
  "test/errors/type/math/gcd3.yasl",
  "test/errors/type/math/isprime.yasl",
  "test/errors/type/math/lcm1.yasl",
  "test/errors/type/math/lcm2.yasl",
  "test/errors/type/math/lcm3.yasl",
  "test/errors/type/math/log.yasl",
  "test/errors/type/math/max1.yasl",
  "test/errors/type/math/max2.yasl",
  "test/errors/type/math/min1.yasl",
  "test/errors/type/math/min2.yasl",
  "test/errors/type/math/rad.yasl",
  "test/errors/type/math/sin.yasl",
  "test/errors/type/math/sqrt.yasl",
  "test/errors/type/math/tan.yasl",
  "test/errors/type/mt/setmt1.yasl",
  "test/errors/type/mt/setmt2.yasl",
  "test/errors/type/str/count1.yasl",
  "test/errors/type/str/count2.yasl",
  "test/errors/type/str/count3.yasl",
  "test/errors/type/str/count4.yasl",
  "test/errors/type/str/endswith1.yasl",
  "test/errors/type/str/endswith2.yasl",
  "test/errors/type/str/endswith3.yasl",
  "test/errors/type/str/endswith4.yasl",
  "test/errors/type/str/isalnum.yasl",
  "test/errors/type/str/isal.yasl",
  "test/errors/type/str/isnum.yasl",
  "test/errors/type/str/isspace.yasl",
  "test/errors/type/str/ltrim1.yasl",
  "test/errors/type/str/ltrim2.yasl",
  "test/errors/type/str/ltrim3.yasl",
  "test/errors/type/str/ltrim4.yasl",
  "test/errors/type/str/rep1.yasl",
  "test/errors/type/str/rep2.yasl",
  "test/errors/type/str/rep3.yasl",
  "test/errors/type/str/rep4.yasl",
  "test/errors/type/str/replace1.yasl",
  "test/errors/type/str/replace2.yasl",
  "test/errors/type/str/replace3.yasl",
  "test/errors/type/str/replace4.yasl",
  "test/errors/type/str/replace5.yasl",
  "test/errors/type/str/replace6.yasl",
  "test/errors/type/str/replace7.yasl",
  "test/errors/type/str/rtrim1.yasl",
  "test/errors/type/str/rtrim2.yasl",
  "test/errors/type/str/rtrim3.yasl",
  "test/errors/type/str/rtrim4.yasl",
  "test/errors/type/str/search2.yasl",
  "test/errors/type/str/search3.yasl",
  "test/errors/type/str/search4.yasl",
  "test/errors/type/str/search.yasl",
  "test/errors/type/str/split1.yasl",
  "test/errors/type/str/split2.yasl",
  "test/errors/type/str/split3.yasl",
  "test/errors/type/str/split4.yasl",
  "test/errors/type/str/startswith2.yasl",
  "test/errors/type/str/startswith3.yasl",
  "test/errors/type/str/startswith4.yasl",
  "test/errors/type/str/startswith.yasl",
  "test/errors/type/str/tobool.yasl",
  "test/errors/type/str/tofloat.yasl",
  "test/errors/type/str/toint.yasl",
  "test/errors/type/str/tolower.yasl",
  "test/errors/type/str/tostr.yasl",
  "test/errors/type/str/toupper.yasl",
  "test/errors/type/str/trim1.yasl",
  "test/errors/type/str/trim2.yasl",
  "test/errors/type/str/trim3.yasl",
  "test/errors/type/str/trim4.yasl",
  "test/errors/type/table/clear.yasl",
  "test/errors/type/table/copy.yasl",
  "test/errors/type/table/keys.yasl",
  "test/errors/type/table/mutable_key2.yasl",
  "test/errors/type/table/mutable_key.yasl",
  "test/errors/type/table/remove.yasl",
  "test/errors/type/table/tostr.yasl",
  "test/errors/type/table/values.yasl",
  "test/errors/type/unary_operators/__bnot.yasl",
  "test/errors/type/unary_operators/__len.yasl",
  "test/errors/type/unary_operators/__neg.yasl",
  "test/errors/type/unary_operators/__pos.yasl",
  "test/errors/type/undef/tobool.yasl",
  "test/errors/type/undef/tostr.yasl",
};
