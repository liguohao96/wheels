const node_env = this?false:true

const toString = function (obj, arg_str) {
    if (typeof (obj) == "object") {
        return obj.toString();
    }
    if (typeof (obj) == "string") {
        return obj;
    }
    if (typeof (obj) == "number") {
        let naive_str = obj.toString();
        if (!arg_str) {
            return naive_str;
        }

        let arg_reg = new RegExp(':([0-9]+)?.([0-9]+)([f])');
        let result = arg_reg.exec(arg_str);

        let int_percision = parseInt(result[1]) || 0;
        let float_percision = parseInt(result[2]) || 0;

        naive_str = obj.toFixed(float_percision);
        let int_reg = new RegExp('([+-])?([0-9]+)((.)([0-9]+))?');
        result = int_reg.exec(naive_str);

        let int_part = result[2];
        let float_part = result[3] || '';
        let sign_part = result[1] || '';

        for (let i = sign_part.length; i < int_percision; ++i) {
            int_part = "0" + int_part;
        }
        int_part = int_part.slice(int_part.length - Math.max(result[2].length, int_percision - sign_part.length), int_part.length);
        return sign_part + int_part + float_part;
    }
}
String.prototype.format = function (...args) {
    var result = this;
    if (arguments.length > 0) {
        let reg = new RegExp("({)(([_a-zA-Z][_a-zA-Z0-9]*)|([0-9]+)|())(:[.a-zA-Z0-9]+)?(}|)", "g");
        let ret = reg.exec(this);
        let last_idx = 0;
        let result_list = [];
        let incremental_idx = 0;
        while (ret) {
            let normal = this.slice(last_idx, ret.index);
            result_list.push(normal);
            if (ret[3]) {
                // key
                let key = ret[3];
                if (args.length == 1 && typeof (args[0]) == "object") {
                    if (key in args[0]) {
                        // format?
                        result_list.push(toString(args[0][key], ret[6]));
                    }
                }
            } else {
                let index = incremental_idx;
                if (ret[4]) {
                    // index
                    index = parseInt(ret[4]);
                } else {
                    // incremental index
                    ++incremental_idx;
                }
                result_list.push(toString(args[index], ret[6]));
            }
            last_idx = ret.index + ret[0].length;
            ret = reg.exec(this);
        };
        if (last_idx < this.length ) {
            result_list.push(this.slice(last_idx, this.length))
        }
        if(node_env){
            console.log('list:');
            console.log(result_list);
        }
        return result_list.join('');
        // if (arguments.length == 1 && typeof (args) == "object") {
        //     for (var key in args) {
        //         if (args[key] != undefined) {
        //             var reg = new RegExp("({" + key + "})", "g");
        //             result = result.replace(reg, args[key]);
        //         }
        //     }
        // } else {
        //     var index = 0;
        //     var ext_reg = new RegExp("({})", "g");
        //     for (var i = 0; i < arguments.length; i++) {
        //         if (arguments[i] != undefined) {
        //             r = new RegExp("({)(([_a-zA-Z][_a-zA-Z0-9]*)|([0-9]+))(:[.a-zA-Z0-9]+)?(}|)");
        //             var reg = new RegExp("({)" + i + "(})", "g");
        //             var replaced = result.replace(reg, arguments[i]);
        //             if (result == replaced) {
        //                 ext_reg.exec(replaced);
        //                 replaced = replaced.slice(0, ext_reg.lastIndex - 2) + arguments[i] + replaced.slice(ext_reg.lastIndex, replaced.length);
        //             }
        //             result = replaced;
        //         }
        //     }
        // }
    }
    return result;
}

if (node_env) {

    let test_list = [{
        args: ['1'],
        string: "{}xxxx",
        result: "1xxxx"
    },
    {
        args: [1.1111111, 3, 4],
        string: "xxx {1} x{:.2f} {2} xxxx",
        result: "xxx 3 x1.11 4 xxxx"
    },
    {
        args: [1.1111111],
        string: "{}%",
        result: "1.1111111%"
    }
    ]
    for (let item of test_list) {
        let res = item.string.format.apply(item.string, item.args);
        if (res == item.result) {
            console.log('good ' + item.string);
        } else {
            console.log('bad ' + item.result + ' vs ' + res);
        }
    }
}