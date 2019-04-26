require('./index.js');
var list_len = 10,
    test_count = 1000,
    err_count = 0;
var array = [];
for (var i = 0; i < list_len; ++i)
    array.push(i);
// array.push(Math.round(Math.random() * 1000));
console.log(array);
for (var i = 0; i < test_count; ++i) {
    var idx = Math.floor(Math.random() * 2 * list_len - list_len);
    if (array[(idx + list_len) % list_len] !== array.get(idx)) {
        err_count += 1;
        console.log("[error] " + idx + " " + array[(idx + list_len) % list_len] + "" + array.get(idx))
    }
}


var test_case = [{
        'index': ':-1',
        'result': [0, 1, 2, 3, 4, 5, 6, 7, 8]
    },
    {
        'index': ':-3',
        'result': [0, 1, 2, 3, 4, 5, 6]
    },
    {
        'index': '5:',
        'result': [5, 6, 7, 8, 9]
    },
    {
        'index': '5:-3',
        'result': [5, 6]
    },
    {
        'index': '5:-3:-1',
        'result': []
    },
    {
        'index': '-1::-1',
        'result': [9, 8, 7, 6, 5, 4, 3, 2, 1, 0]
    },
    {
        'index': '-1:-2:-1',
        'result': [9]
    },
    {
        'index': '-1:-2:1',
        'result': []
    },
    {
        'index': '::-1',
        'result': [9, 8, 7, 6, 5, 4, 3, 2, 1, 0]
    },
    {
        'index': [1, 2],
        'result': [1, 2]
    },
    {
        'index': [true, false, false, false, false, false, false, false, false, false, ],
        'result': [0]
    },
];

for (let tc of test_case) {
    let ret = array.get(tc.index);
    if (ret.length == tc.result.length) {
        for (let i = 0; i < ret.length; ++i) {
            if (ret[i] !== tc.result[i]) {
                err_count += 1;
                console.log("[error] '" + tc.index + "' " + JSON.stringify(ret) + ' ' + JSON.stringify(tc.result));
                break;
            }
        }
    } else {
        err_count += 1;
        console.log("[error] '" + tc.index + "' " + JSON.stringify(ret) + ' ' + JSON.stringify(tc.result));
    }
}

console.log('err count: ' + err_count);