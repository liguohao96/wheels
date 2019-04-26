var Int = (function () {
    var NotAIntegerException = function (value) {
        var ret = new Error(`${value} is Not a integer`);
        ret.name = 'NotAIntegerException';
        return ret;
    };
    var OutOfRange = function (value, min, max) {
        var ret = new Error(`${value} is not in [${min}, ${max})`);
        ret.name = 'OutOfRangeException';
        return ret;
    };
    var ArgumentLengthException = function (expect_len, actual_len) {
        var err = new Error(`expected ${expect_len}, but got ${actual_len}`);
        err.name = 'ArgumentLengthException';
        return err;
    }
    var isInteger = Number.isInteger || function (value) {
        'use strict';
        return typeof value == 'number' &&
            isFinite(value) &&
            Math.floor(value) === value;
    }

    function constructor() {
        'use strict';
        if (!this || (typeof window !== 'undefined' && this == window)) {
            return new constructor(...arguments);
        }
        this._min = Number.NEGATIVE_INFINITY;
        this._max = Number.POSITIVE_INFINITY;
        var arg;
        if (arguments[0]) {
            // try to parse to integer
            arg = arguments[0];
            if (isInteger(arg)) {
                this._min = arg;
            }
        }
        if (arguments[1]) {
            // try to parse to integer
            arg = arguments[1];
            if (isInteger(arg)) {
                this._max = arg;
            }
        }
        return this;
    }
    constructor.prototype.parse = function (value) {
        'use strict';
        if (arguments.length != 1) {
            return [Number.NaN, ArgumentLengthException(1, arguments.length)];
        }
        value = Number(value);
        if (!isInteger(value))
            return [Number.NaN, NotAIntegerException(value)];
        if (this._min <= value && value < this._max) {
            return [value, null];
        } else {
            return [Number.NaN, OutOfRangeException(value, this._min, this._max)];
        }
    }
    return constructor
})();

Array.prototype._resolve_index = function (idx) {
    'use strict';
    var is_single = true;
    var start = 0,
        end = this.length,
        step = 1;
    var index, err, idx_list = [];
    var valid_idx_template = new Int(-this.length, this.length);
    if (typeof idx == 'string') {
        // parse idx as string
        idx = idx.trim();
        var splited = idx.split(':');
        var number;
        is_single = splited.length == 1 ? true : false;
        if (splited.length == 3) {
            [step, err] = valid_idx_template.parse(splited[2]);
            if (err)
                throw err;
            if (step === 0) {
                throw new Error("step can't be 0");
            }
            if (step < 0) {
                start = this.length - 1, end = -1;
            }
        }
        for (var i = 0, len = Math.min(splited.length, 2); i < len; ++i) {
            if (splited[i].length === 0) {
                continue;
            }
            switch (i) {
                case 0:
                    {
                        [start, err] = valid_idx_template.parse(splited[i]);
                        start = (start + this.length) % this.length;
                        index = start;
                        break;
                    }
                case 1:
                    {
                        [end, err] = valid_idx_template.parse(splited[i]);
                        end = (end + this.length) % this.length;
                        break;
                    }
            }
            if (err) {
                throw err;
            }
        }
        if (step > 0 && start - end <= 0) {
            for (var i = start, len = this.length; i < len && i < end; i += step) {
                idx_list.push(i);
            }
        } else if (step < 0 && start - end > 0) {
            for (var i = start; i >= 0 && i > end; i += step) {
                idx_list.push(i);
            }
        }
    } else if (idx instanceof Array) {
        is_single = false;
        if (idx.length > 0) {
            if (typeof idx[0] === 'boolean') {
                if (idx.length == this.length) {
                    for (var i = 0, len = idx.length; i < len; ++i) {
                        if (idx[i])
                            idx_list.push(i);
                    }
                } else {
                    throw new Error('mask length did not equal to length of Array');
                }
            } else {
                for (var i = 0, len = idx.length; i < len; ++i) {
                    [index, err] = valid_idx_template.parse(idx[i]);
                    index = (index + this.length) % this.length;
                    idx_list.push(index);
                    if (err)
                        throw err;
                }
            }
        }
    } else if (typeof idx == 'number') {
        // parse idx as integer
        [index, err] = valid_idx_template.parse(idx);
        if (!err) {
            index = (index + this.length) % this.length;
        } else {
            throw err;
        }
    }
    if (is_single) {
        return index;
    } else {
        return idx_list;
    }
}

Array.prototype.set = function (idx, values) {
    'use strict';
    var index = this._resolve_index(idx);
    if (typeof index == 'number') {
        this[index] = values;
    } else {
        if (values instanceof Array) {
            if (index.length === values.length) {
                for (var i = 0, len = index.length; i < len; ++i) {
                    this[index[i]] = values[i];
                }
            } else {
                throw new Error('value length did not equal to number of item selected');
            }
        } else {
            for (var i = 0, len = index.length; i < len; ++i) {
                // TODO: copy
                this[index[i]] = values;
            }
        }
    }
    return this;
}

Array.prototype.get = function (idx) {
    'use strict';
    var index = this._resolve_index(idx);
    if (typeof index == 'number') {
        return this[index];
    } else {
        var items = [];
        for (var i = 0, len = index.length; i < len; ++i) {
            items.push(this[index[i]]);
        }
        return items;
    }
}