/**
 * version: 0.1
 * author: Guohao Li
 * email: liguohao96@163.com 
 */
function is_obj_equ(lhs, rhs) {
  if (typeof lhs != typeof rhs) {
    return false;
  }
  if (lhs instanceof Array) {
    if (lhs.length !== rhs.length) {
      return false;
    }
    for (let i in lhs) {
      if (is_obj_equ(lhs[i], rhs[i])) {

      } else {
        return false
      }
    }
    return true;
  }
  if (!rhs || !lhs) {
    return rhs == lhs;
  }
  for (let k in lhs) {
    if (k in rhs && lhs[k] == rhs[k]) {

    } else {
      return false;
    }
  }
  return true;
}

function copy_obj(obj) {
  return JSON.parse(JSON.stringify(obj))
}

function createJSONFunction(method) {
  method = method.toUpperCase();
  return function (url, json_obj, callback) {
    if (callback === undefined || callback === null) {
      console.warn("callback is undefined")
    } else {
      if (!typeof (callback) == 'function') {
        var err = Error("a function is required, but parsed " + typeof (
          callback));
        throw err;
      }
    }

    let xhr = new XMLHttpRequest();
    xhr.onreadystatechange = () => {
      if (xhr.readyState == 4) {
        if (xhr.status == 200) {
          let ret = xhr.response;
          try {
            ret = JSON.parse(ret);
          } catch (e) {
            ret = xhr.response;
          }
          if (callback) {
            // callback may be undefined or null which is uncallable
            callback(ret, undefined);
          }
        }
      }
    }

    if (url.indexOf("http") == -1) {
      if (api_url) {
        url = api_url + url;
      }
    }
    if (method === 'GET') {
      let extension_str = "?"
      if (json_obj) {
        for (let key in json_obj) {
          extension_str += key + "=" + json_obj[key] + "&"
        }
      }
      extension_str = extension_str.slice(0, -1);
      url = url + extension_str;
      json_obj = null;
    }
    xhr.open(method, url, true);
    xhr.responseType = this.responseType || 'text';
    const headers = this.headers;
    for (let v in headers) {
      xhr.setRequestHeader(v, headers[v]);
    }
    xhr.withCredentials = this.withCredentials;
    if (json_obj)
      xhr.send(JSON.stringify(json_obj));
    else
      xhr.send();
  }
}

const AjaxGet = createJSONFunction('get');
const AjaxPost = createJSONFunction('post');
const AjaxPut = createJSONFunction('put');
const AjaxDelete = createJSONFunction('delete');

function createPromisedJSONFunction(method) {
  method = method.toUpperCase();
  return function (url, json_obj, resolve, reject) {
    let ret = new Promise((res, rej) => {

      let xhr = new XMLHttpRequest();
      xhr.onreadystatechange = () => {
        if (xhr.readyState == 4) {
          if (xhr.status == 200) {
            console.log(xhr);
            let ret = xhr.response;
            try {
              ret = JSON.parse(ret);
            } catch (e) {
              ret = xhr.response;
            }
            res(ret);
          } else {
            rej(new Error("XMLHttpRequest status:" + xhr.status + "\n" + xhr.responseText));
          }
        }
      }

      if (url.indexOf("http") == -1) {
        if (api_url) {
          url = api_url + url;
        }
      }
      if (method === 'GET') {
        let extension_str = "?"
        if (json_obj) {
          for (let key in json_obj) {
            extension_str += key + "=" + json_obj[key] + "&"
          }

        }
        extension_str = extension_str.slice(0, -1);
        url = url + extension_str;
        json_obj = null;
      }

      xhr.open(method, url, true);
      xhr.responseType = this.responseType || 'text';
      const headers = this.headers;
      for (let v in headers) {
        xhr.setRequestHeader(v, headers[v]);
      }
      xhr.withCredentials = this.withCredentials;
      if (json_obj) {
        xhr.send(JSON.stringify(json_obj));
      } else {
        xhr.send();
      }
    });
    ret = ret.then(resolve).catch(reject);
    return ret;
  }
}
const AjaxPromisedGet = createPromisedJSONFunction('get');
const AjaxPromisedPost = createPromisedJSONFunction('post');
const AjaxPromisedPut = createPromisedJSONFunction('put');
const AjaxPromisedDelete = createPromisedJSONFunction('delete');
const Ajax = (
  function () {
    // define global cache for return value
    const __global_pool = new Object();
    __global_pool.__url_map = new Map();
    __global_pool.set = (function (url, method, req, value) {
      value = copy_obj(value);
      let item = this.__url_map.get(url);
      let ret = null;
      if (item) {
        const stored_req_list = item[method];
        for (let idx in stored_req_list) {
          if (is_obj_equ(stored_req_list[idx], req)) {
            return stored_req_list[idx];
          }
        }
        ret = {
          "req": req,
          "value": value
        };
        stored_req_list.push(ret);
      } else {
        item = {
          "GET": [],
          "POST": [],
          "PUT": [],
          "DELETE": []
        };
        ret = {
          "req": req,
          "value": value
        };
        item[method].push(ret);
        this.__url_map.set(url, item);
      }
      return ret;
    });
    __global_pool.get = (function (url, method, req) {
      let item = this.__url_map.get(url);
      let ret = null;
      if (item) {
        const stored_req_list = item[method];
        for (let idx in stored_req_list) {
          if (is_obj_equ(stored_req_list[idx].req, req)) {
            return stored_req_list[idx].value;
          }
        }
      }
      return ret;
    })

    __global_pool.delete = (function (url, method, req) {
      let item = this.__url_map.get(url);
      let ret = false;
      if (item) {
        if (!req) {
          item[method] = [];
        }
        const stored_req_list = item[method];
        for (let idx in stored_req_list) {
          if (is_obj_equ(stored_req_list[idx].req, req)) {
            stored_req_list.splice(idx, 1);
            ret = true;
            break;
          }
        }
      }
      return ret;
    })

    // define default values
    const __type_enum = ['callback', 'promise'];
    let __default_type = __type_enum[0];
    let __default_resolve = function (data) {
      return data;
    }
    let __default_reject = function (msg) {
      console.warn('reject has occurd')
      console.warn(msg);
    }
    const __default_options = {
      headers: {
        "content-type": "application/json"
      },
      withCredentials: true,
      responseType: '',
      cache: true,
      debug: false
    };

    function constructor(type, options) {
      if (this) {
        this._type = type || __default_type;
        options = options || {};
        this._headers = options.headers || __default_options.headers;
        this._with_credentials = 'withCredentials' in options ? options.withCredentials : __default_options.withCredentials;
        this._responseType = 'responseType' in options ? options.responseType : __default_options.responseType;
        this._cache = 'cache' in options ? options.cache : __default_options.cache;
        this._debug = 'debug' in options ? options.debug : __default_options.debug;
        const _cache = {
          delete: function (url, method, req) {
            __global_pool.delete(url, method, req);
          }
        };
        const cache_handler = {};
        this.cache = new Proxy(_cache, cache_handler);
        return this;
      } else {
        return new constructor(type, options);
      }
      return this;
    }
    constructor.prototype.__defineSetter__("default_type", function (type) {
      type = type.toLowerCase();
      if (__type_enum.indexOf(type) >= 0) {
        __default_type = type;
      }
    });
    constructor.prototype.__defineSetter__("default_options", function (options) {
      for (let k in __default_options) {
        if (k in options) {
          __default_options[k] = options[k];
        }
      }
    });
    constructor.prototype.__defineGetter__("headers", function () {
      return this._headers;
    });
    constructor.prototype.__defineGetter__("withCredentials", function () {
      return this._with_credentials;
    });
    constructor.prototype.__defineGetter__("responseType", function () {
      return this._responseType;
    });

    // request function
    constructor.prototype.get = function (url, data, ...args) {
      const cc = {
        "callback": (function (callback) {
          return (function (ret, err) {
            if (err) {
              callback(ret, err)
            } else {
              if (ret && ret.status && ret.status == "succ") {
                __global_pool.set(url, "GET", data, ret);
              }
              callback(ret, null);
            }
          }).bind(this);
        }).bind(this),
        "promise": (function () {
          return (function (ret) {
            if (ret && ret.status && ret.status == "succ") {
              if (this._debug) {
                console.log("store [GET]" + url + " with req " + JSON.stringify(data) + " in cache");
                console.log("data is " + JSON.stringify(ret));
              }
              __global_pool.set(url, "GET", data, ret);
            }
            return ret;
          }).bind(this);
        }).bind(this)
      }
      if (this._type == __type_enum[0]) {
        // callback
        const callback_fn = args[0];
        if (this._cache) {
          AjaxGet.call(this, url, data, cc.callback(callback_fn));
        } else {
          AjaxGet.call(this, url, data, callback_fn);
        }
      } else {
        const resolve = args[0] || __default_resolve,
          reject = args[1] || __default_reject;

        let promise;
        if (this._cache) {
          // determin if cached
          const cached_ret = __global_pool.get(url, "GET", data);
          if (cached_ret) {
            // console.log("using cached");
            if (this._debug) {
              console.log("cache hit with [GET]" + url + " with " + JSON.stringify(data));
              console.log("ret is " + JSON.stringify(cached_ret));
            }
            const new_ajax = this.copy();
            let new_p = new Promise((res, rej) => {
              res(cached_ret);
            });
            new_p = new_p.then(resolve).catch(reject);
            new_ajax._promise = new_p;
            return new_ajax;
          }
          promise = AjaxPromisedGet.call(this, url, data, cc.promise(), reject);
          promise = promise.then(resolve);
        } else {
          promise = AjaxPromisedGet.call(this, url, data, resolve, reject);
        }
        if (!this._promise) {
          this._promise = promise;
        } else {
          this._promise = this._promise.then(promise);
        }
        return this;
      }
    };
    constructor.prototype.post = function (url, data, ...args) {
      if (this._type == __type_enum[0]) {
        const callback_fn = args[0];
        AjaxPost.call(this, url, data, callback_fn);
        return this;
      } else {
        const resolve = args[0] || __default_resolve,
          reject = args[1] || __default_reject;
        const promise = AjaxPromisedPost.call(this, url, data, resolve, reject);
        if (!this._promise) {
          this._promise = promise;
        } else {
          this._promise = this._promise.then(promise);
        }
        return this;
      }
    };
    constructor.prototype.put = function (url, data, ...args) {
      if (this._type == __type_enum[0]) {
        const callback_fn = args[0];
        AjaxPut.call(this, url, data, callback_fn);
        return this;
      } else {
        const resolve = args[0] || __default_resolve,
          reject = args[1] || __default_reject;
        const promise = AjaxPromisedPut.call(this, url, data, resolve, reject);
        if (!this._promise) {
          this._promise = promise;
        } else {
          this._promise = this._promise.then(promise);
        }
        return this;
      }
    };
    constructor.prototype.delete = function (url, data, ...args) {
      if (this._type == __type_enum[0]) {
        const callback_fn = args[0];
        AjaxDelete.call(this, url, data, callback_fn);
      } else {
        const resolve = args[0] || __default_resolve,
          reject = args[1] || __default_reject;
        const promise = AjaxPromisedDelete.call(this, url, data, resolve, reject);
        if (!this._promise) {
          this._promise = promise;
        } else {
          this._promise = this._promise.then(promise);
        }
        return this;
      }
    };
    constructor.prototype.upload = function (url, data, ...args) {
      // expect data to be a FormData

      let finish_func, err_func, progress_func;
      const default_progress_func = (ret) => {
        console.log(ret);
      }
      const default_err_func = default_progress_func;
      let promise_type = false;
      let xhr = new XMLHttpRequest();
      if (this._type == __type_enum[0]) {
        // callback
        finish_func = args[0];
        progress_func = args[1] || default_progress_func;

        if (!data instanceof FormData) {
          finish_func(undefined, new TypeError("expeted data to be a FormData but got " + data.constructor));
        }

        xhr.onreadystatechange = () => {
          if (xhr.readyState == 4) {
            if (xhr.status == 200) {
              let ret = xhr.responseText;
              try {
                ret = JSON.parse(ret);
              } catch (e) {
                ret = xhr.responseText;
              }
              if (finish_func) {
                // callback may be undefined or null which is uncallable
                finish_func(ret, undefined);
              }
            }
          }
        }
        // err_func.args
      } else {
        // promise
        finish_func = args[0];
        progress_func = args[1] || default_progress_func;
        err_func = args[2] || default_err_func;
        let ret_p = new Promise((res, rej) => {

          if (!data instanceof FormData) {
            rej(new TypeError("expeted data to be a FormData but got " + data.constructor));
          }

          xhr.onreadystatechange = (event) => {
            if (xhr.readyState == 4) {
              if (xhr.status == 200) {
                let ret = xhr.responseText;
                try {
                  ret = JSON.parse(ret);
                } catch (e) {
                  ret = xhr.responseText;
                }
                if (res) {
                  res(ret);
                }
              } else {
                if (rej) {
                  rej(Error("XMLHttpRequest status:" + xhr.status + "\n" + xhr.responseText));
                }
              }
            }
          }
        });
        ret_p = ret_p.then(finish_func).catch(err_func);
        if (this._promise) {
          this._promise.then(ret_p)
        } else
          this._promise = ret_p;
      }
      xhr.upload.onprogress = (event) => {
        let percent = event.loaded / event.total;
        progress_func(percent);
      };
      xhr.upload.onloadstart = (event) => {
        progress_func(0.0);
      };
      if (url.indexOf("http") == -1) {
        if (api_url) {
          url = api_url + url;
        }
      }
      xhr.open("POST", url, true);
      // xhr.setRequestHeader('Content-type', 'multipart/form-data');
      // xhr.withCredentials = true;
      if (data) {
        xhr.send(data);
      } else {
        throw new Error("payload required");
      }
      return this;
    };

    // chain function
    constructor.prototype.then = function (promise_obj) {
      if (this._type == __type_enum[1]) {
        if (this._promise) {
          this._promise = this._promise.then(promise_obj);
        }
      }
      return this;
    }

    // support function
    constructor.prototype.copy = function () {
      const new_ajax = new constructor(this._type, {
        "header": this._headers,
        "withCredentials": this._with_credentials,
        "cache": this._cache,
        "debug": this._debug
      });
      new_ajax._promise = this._promise;
      return new_ajax
    };

    return constructor;
  }
)();

const api_url = null; // default url