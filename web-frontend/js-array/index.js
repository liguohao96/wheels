var Int = (function () {
    var NotAInteger = new Error('Not a integer');
    var OutOfRange = new Error('Out of range');
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
        if (!value && !isInteger(value))
            return NotAInteger;
        if (this._min <= value && value < this._max) {
            return value;
        } else {
            return OutOfRange;
        }
    }
    return constructor
})();

Array.prototype.set = function () {

}

Array.prototype.get = function (idx) {
    var is_single = true;
    var start = 0, end = this.length - 1, step = 1;
    var index, copy_list;
    if (typeof idx == 'string') {
        // parse idx as string
        idx = idx.trim();
        var splited = idx.split(':');
        var number;
        for (var i = 0, len = Math.min(splited.length, 3); i < len; ++i) {
            number = new Number(splited[i]);
            switch (i) {
                case 0: {
                    Int().parse(number);
                }
                case 1: {
                    Int().parse(number);
                }
                case 2: {
                    Int().parse(number);
                }
            }
        }
        // ::-1 => 0:end:-1
        // -3: => -3:end:1
        // 
    } else if (typeof idx == 'number') {
        // parse idx as integer
        var template = new Int(-this.length, this.length);
        index = template.parse(idx);
        if (typeof index == 'number') {
            index = (index + this.length) % this.length;
        } else {
            throw index;
        }
    }

    if (is_single) {
        return this[index];
    } else {
        return copy_list;
    }

}