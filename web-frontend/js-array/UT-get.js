require('./index.js');
var list_len = 100, test_count = 1000, err_count = 0;
var array = [];
for (var i = 0; i < list_len; ++i)
    array.push(Math.round(Math.random() * 1000));

for (var i = 0; i < test_count; ++i) {
    var idx = Math.floor(Math.random() * 2 * list_len - list_len);
    if (array[(idx + list_len) % list_len] !== array.get(idx)) {
        err_count += 1;
        console.log("[error] " + idx + " " + array[(idx + list_len) % list_len] + "" + array.get(idx))
    }
}
console.log('err count: ' + err_count);