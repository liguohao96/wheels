require('./index.js');
var list_len = 10,
    test_count = 1000,
    err_count = 0;
var origin_array = [];
for (var i = 0; i < list_len; ++i)
    origin_array.push(i);
// array.push(Math.round(Math.random() * 1000));
console.log(origin_array);

var test_case = [{
        'argument': [':-1', 4],
        'result': [4, 4, 4, 4, 4, 4, 4, 4, 4, 9]
    },
    {
        'argument': [':-3', 4],
        'result': [4, 4, 4, 4, 4, 4, 4, 7, 8, 9]
    },
    {
        'argument': [':5:-1', 4],
        'result': [0, 1, 2, 3, 4, 5, 4, 4, 4, 4]
    },
    {
        'argument': [
            [true, false, false, false, false, false, false, false, false, false, ],
            10
        ],
        'result': [10, 1, 2, 3, 4, 5, 6, 7, 8, 9]
    },
];

for (let tc of test_case) {
    var array = new Array(...origin_array);
    let ret = array.set(...tc.argument);
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