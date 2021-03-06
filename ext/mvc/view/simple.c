
/*
  +------------------------------------------------------------------------+
  | Phalcon Framework                                                      |
  +------------------------------------------------------------------------+
  | Copyright (c) 2011-2014 Phalcon Team (http://www.phalconphp.com)       |
  +------------------------------------------------------------------------+
  | This source file is subject to the New BSD License that is bundled     |
  | with this package in the file docs/LICENSE.txt.                        |
  |                                                                        |
  | If you did not receive a copy of the license and are unable to         |
  | obtain it through the world-wide-web, please send an email             |
  | to license@phalconphp.com so we can send you a copy immediately.       |
  +------------------------------------------------------------------------+
  | Authors: Andres Gutierrez <andres@phalconphp.com>                      |
  |          Eduar Carvajal <eduar@phalconphp.com>                         |
  +------------------------------------------------------------------------+
*/

#include "mvc/view/simple.h"
#include "mvc/view/exception.h"
#include "mvc/view/engineinterface.h"
#include "mvc/view/engine/php.h"
#include "cache/backendinterface.h"
#include "di/injectable.h"

#include <Zend/zend_closures.h>

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/object.h"
#include "kernel/exception.h"
#include "kernel/operators.h"
#include "kernel/fcall.h"
#include "kernel/array.h"
#include "kernel/hash.h"
#include "kernel/concat.h"
#include "kernel/file.h"
#include "kernel/string.h"
#include "kernel/output.h"
#include "kernel/debug.h"

/**
 * Phalcon\Mvc\View\Simple
 *
 * This component allows to render views without hicherquical levels
 *
 *<code>
 * $view = new Phalcon\Mvc\View\Simple();
 * echo $view->render('templates/my-view', array('content' => $html));
 *</code>
 *
 */
zend_class_entry *phalcon_mvc_view_simple_ce;

PHP_METHOD(Phalcon_Mvc_View_Simple, __construct);
PHP_METHOD(Phalcon_Mvc_View_Simple, setViewsDir);
PHP_METHOD(Phalcon_Mvc_View_Simple, getViewsDir);
PHP_METHOD(Phalcon_Mvc_View_Simple, registerEngines);
PHP_METHOD(Phalcon_Mvc_View_Simple, getRegisteredEngines);
PHP_METHOD(Phalcon_Mvc_View_Simple, _loadTemplateEngines);
PHP_METHOD(Phalcon_Mvc_View_Simple, _internalRender);
PHP_METHOD(Phalcon_Mvc_View_Simple, render);
PHP_METHOD(Phalcon_Mvc_View_Simple, partial);
PHP_METHOD(Phalcon_Mvc_View_Simple, setCacheOptions);
PHP_METHOD(Phalcon_Mvc_View_Simple, getCacheOptions);
PHP_METHOD(Phalcon_Mvc_View_Simple, _createCache);
PHP_METHOD(Phalcon_Mvc_View_Simple, getCache);
PHP_METHOD(Phalcon_Mvc_View_Simple, cache);
PHP_METHOD(Phalcon_Mvc_View_Simple, setParamToView);
PHP_METHOD(Phalcon_Mvc_View_Simple, setVars);
PHP_METHOD(Phalcon_Mvc_View_Simple, setVar);
PHP_METHOD(Phalcon_Mvc_View_Simple, getParamsToView);
PHP_METHOD(Phalcon_Mvc_View_Simple, setContent);
PHP_METHOD(Phalcon_Mvc_View_Simple, getContent);
PHP_METHOD(Phalcon_Mvc_View_Simple, getActiveRenderPath);
PHP_METHOD(Phalcon_Mvc_View_Simple, __set);
PHP_METHOD(Phalcon_Mvc_View_Simple, __get);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_view_simple___construct, 0, 0, 0)
	ZEND_ARG_INFO(0, options)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_view_simple_setviewsdir, 0, 0, 1)
	ZEND_ARG_INFO(0, viewsDir)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_view_simple_registerengines, 0, 0, 1)
	ZEND_ARG_INFO(0, engines)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_view_simple_render, 0, 0, 1)
	ZEND_ARG_INFO(0, path)
	ZEND_ARG_INFO(0, params)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_view_simple_partial, 0, 0, 1)
	ZEND_ARG_INFO(0, partialPath)
	ZEND_ARG_INFO(0, params)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_view_simple_setcacheoptions, 0, 0, 1)
	ZEND_ARG_INFO(0, options)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_view_simple_cache, 0, 0, 0)
	ZEND_ARG_INFO(0, options)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_view_simple_setparamtoview, 0, 0, 2)
	ZEND_ARG_INFO(0, key)
	ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_view_simple_setvars, 0, 0, 1)
	ZEND_ARG_INFO(0, params)
	ZEND_ARG_INFO(0, merge)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_view_simple_setvar, 0, 0, 2)
	ZEND_ARG_INFO(0, key)
	ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_view_simple_setcontent, 0, 0, 1)
	ZEND_ARG_INFO(0, content)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_view_simple___set, 0, 0, 2)
	ZEND_ARG_INFO(0, key)
	ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_view_simple___get, 0, 0, 1)
	ZEND_ARG_INFO(0, key)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_mvc_view_simple_method_entry[] = {
	PHP_ME(Phalcon_Mvc_View_Simple, __construct, arginfo_phalcon_mvc_view_simple___construct, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_ME(Phalcon_Mvc_View_Simple, setViewsDir, arginfo_phalcon_mvc_view_simple_setviewsdir, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View_Simple, getViewsDir, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View_Simple, registerEngines, arginfo_phalcon_mvc_view_simple_registerengines, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View_Simple, getRegisteredEngines, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View_Simple, _loadTemplateEngines, NULL, ZEND_ACC_PROTECTED)
	PHP_ME(Phalcon_Mvc_View_Simple, _internalRender, NULL, ZEND_ACC_PROTECTED)
	PHP_ME(Phalcon_Mvc_View_Simple, render, arginfo_phalcon_mvc_view_simple_render, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View_Simple, partial, arginfo_phalcon_mvc_view_simple_partial, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View_Simple, setCacheOptions, arginfo_phalcon_mvc_view_simple_setcacheoptions, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View_Simple, getCacheOptions, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View_Simple, _createCache, NULL, ZEND_ACC_PROTECTED)
	PHP_ME(Phalcon_Mvc_View_Simple, getCache, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View_Simple, cache, arginfo_phalcon_mvc_view_simple_cache, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View_Simple, setParamToView, arginfo_phalcon_mvc_view_simple_setparamtoview, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View_Simple, setVars, arginfo_phalcon_mvc_view_simple_setvars, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View_Simple, setVar, arginfo_phalcon_mvc_view_simple_setvar, ZEND_ACC_PUBLIC)
	PHP_MALIAS(Phalcon_Mvc_View_Simple, getVar, __get, arginfo_phalcon_mvc_view_simple___get, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View_Simple, getParamsToView, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View_Simple, setContent, arginfo_phalcon_mvc_view_simple_setcontent, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View_Simple, getContent, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View_Simple, getActiveRenderPath, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View_Simple, __set, arginfo_phalcon_mvc_view_simple___set, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_View_Simple, __get, arginfo_phalcon_mvc_view_simple___get, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\Mvc\View\Simple initializer
 */
PHALCON_INIT_CLASS(Phalcon_Mvc_View_Simple){

	PHALCON_REGISTER_CLASS_EX(Phalcon\\Mvc\\View, Simple, mvc_view_simple, phalcon_di_injectable_ce, phalcon_mvc_view_simple_method_entry, 0);

	zend_declare_property_null(phalcon_mvc_view_simple_ce, SL("_options"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_view_simple_ce, SL("_viewsDir"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_view_simple_ce, SL("_partialsDir"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_view_simple_ce, SL("_viewParams"), ZEND_ACC_PROTECTED);
	zend_declare_property_bool(phalcon_mvc_view_simple_ce, SL("_engines"), 0, ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_view_simple_ce, SL("_registeredEngines"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_view_simple_ce, SL("_activeRenderPath"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_view_simple_ce, SL("_content"), ZEND_ACC_PROTECTED);
	zend_declare_property_bool(phalcon_mvc_view_simple_ce, SL("_cache"), 0, ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_view_simple_ce, SL("_cacheOptions"), ZEND_ACC_PROTECTED);

	return SUCCESS;
}

/**
 * Phalcon\Mvc\View constructor
 *
 * @param array $options
 */
PHP_METHOD(Phalcon_Mvc_View_Simple, __construct){

	zval *options = NULL;

	phalcon_fetch_params(0, 0, 1, &options);

	if (options && Z_TYPE_P(options) == IS_ARRAY) {
		phalcon_update_property_this(getThis(), SL("_options"), options);
	}
}

/**
 * Sets views directory. Depending of your platform, always add a trailing slash or backslash
 *
 * @param string $viewsDir
 */
PHP_METHOD(Phalcon_Mvc_View_Simple, setViewsDir){

	zval *views_dir;

	phalcon_fetch_params(0, 1, 0, &views_dir);

	phalcon_update_property_this(getThis(), SL("_viewsDir"), views_dir);

}

/**
 * Gets views directory
 *
 * @return string
 */
PHP_METHOD(Phalcon_Mvc_View_Simple, getViewsDir){


	RETURN_MEMBER(getThis(), "_viewsDir");
}

/**
 * Register templating engines
 *
 *<code>
 *$this->view->registerEngines(array(
 *  ".phtml" => "Phalcon\Mvc\View\Engine\Php",
 *  ".mhtml" => "MyCustomEngine"
 *));
 *</code>
 *
 * @param array $engines
 */
PHP_METHOD(Phalcon_Mvc_View_Simple, registerEngines){

	zval *engines;

	phalcon_fetch_params(0, 1, 0, &engines);

	if (Z_TYPE_P(engines) != IS_ARRAY) { 
		PHALCON_THROW_EXCEPTION_STRW(phalcon_mvc_view_exception_ce, "Engines to register must be an array");
		return;
	}
	phalcon_update_property_this(getThis(), SL("_registeredEngines"), engines);

}

/**
 * Returns the registered templating engines
 *
 * @brief array Phalcon\Mvc\View\Simple::getRegisteredEngines()
 */
PHP_METHOD(Phalcon_Mvc_View_Simple, getRegisteredEngines) {

	RETURN_MEMBER(getThis(), "_registeredEngines")
}

/**
 * Loads registered template engines, if none is registered it will use Phalcon\Mvc\View\Engine\Php
 *
 * @return array
 */
PHP_METHOD(Phalcon_Mvc_View_Simple, _loadTemplateEngines){

	zval engines = {}, *dependency_injector, *registered_engines, php_engine = {}, arguments = {}, *engine_service;
	zend_string *str_key;
	ulong idx;

	phalcon_return_property(&engines, getThis(), SL("_engines"));

	/** 
	 * If the engines aren't initialized 'engines' is false
	 */
	if (!zend_is_true(&engines)) {
		array_init(&engines);

		dependency_injector = phalcon_read_property(getThis(), SL("_dependencyInjector"), PH_NOISY);

		registered_engines = phalcon_read_property(getThis(), SL("_registeredEngines"), PH_NOISY);
		if (Z_TYPE_P(registered_engines) != IS_ARRAY) { 
			/** 
			 * We use Phalcon\Mvc\View\Engine\Php as default
			 */
			object_init_ex(&php_engine, phalcon_mvc_view_engine_php_ce);
			PHALCON_CALL_METHODW(NULL, &php_engine, "__construct", getThis(), dependency_injector);

			/** 
			 * Use .phtml as extension for the PHP engine
			 */
			phalcon_array_update_str(&engines, SL(".phtml"), &php_engine, PH_COPY);
		} else {
			if (Z_TYPE_P(dependency_injector) != IS_OBJECT) {
				PHALCON_THROW_EXCEPTION_STRW(phalcon_mvc_view_exception_ce, "A dependency injector container is required to obtain the application services");
				return;
			}

			/** 
			 * Arguments for instantiated engines
			 */
			array_init_size(&arguments, 2);
			phalcon_array_append(&arguments, getThis(), PH_COPY);
			phalcon_array_append(&arguments, dependency_injector, PH_COPY);

			ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(registered_engines), idx, str_key, engine_service) {
				zval extension = {}, engine_object = {}, exception_message = {};
				if (str_key) {
					ZVAL_STR(&extension, str_key);
				} else {
					ZVAL_LONG(&extension, idx);
				}
				if (Z_TYPE_P(engine_service) == IS_OBJECT) {

					/** 
					 * Engine can be a closure
					 */
					if (instanceof_function(Z_OBJCE_P(engine_service), zend_ce_closure)) {
						PHALCON_CALL_USER_FUNC_ARRAYW(&engine_object, engine_service, &arguments);
					} else {
						PHALCON_CPY_WRT(&engine_object, engine_service);
					}
				} else if (Z_TYPE_P(engine_service) == IS_STRING) {
					/** 
					 * Engine can be a string representing a service in the DI
					 */
					PHALCON_CALL_METHODW(&engine_object, dependency_injector, "getshared", engine_service, &arguments);
					PHALCON_VERIFY_INTERFACEW(&engine_object, phalcon_mvc_view_engineinterface_ce);
				} else {
					PHALCON_CONCAT_SV(&exception_message, "Invalid template engine registration for extension: ", &extension);
					PHALCON_THROW_EXCEPTION_ZVALW(phalcon_mvc_view_exception_ce, &exception_message);
					return;
				}
				phalcon_array_update_zval(&engines, &extension, &engine_object, PH_COPY);
			} ZEND_HASH_FOREACH_END();

		}

		phalcon_update_property_this(getThis(), SL("_engines"), &engines);
	}

	RETURN_CTORW(&engines);
}

/**
 * Tries to render the view with every engine registered in the component
 *
 * @param string $path
 * @param array $params
 */
PHP_METHOD(Phalcon_Mvc_View_Simple, _internalRender){

	zval *path, *params, *events_manager, event_name = {}, status = {}, debug_message = {};
	zval not_exists = {}, *views_dir, views_dir_path = {}, engines = {}, *engine, exception_message = {};
	zend_string *str_key;
	ulong idx;

	phalcon_fetch_params(0, 2, 0, &path, &params);

	events_manager = phalcon_read_property(getThis(), SL("_eventsManager"), PH_NOISY);
	if (Z_TYPE_P(events_manager) == IS_OBJECT) {
		phalcon_update_property_this(getThis(), SL("_activeRenderPath"), path);
	}

	/** 
	 * Call beforeRender if there is an events manager
	 */
	if (Z_TYPE_P(events_manager) == IS_OBJECT) {
		ZVAL_STRING(&event_name, "view:beforeRender");

		PHALCON_CALL_METHODW(&status, events_manager, "fire", &event_name, getThis());
		if (PHALCON_IS_FALSE(&status)) {
			RETURN_NULL();
		}
	}

	if (unlikely(PHALCON_GLOBAL(debug).enable_debug)) {
		PHALCON_CONCAT_SV(&debug_message, "Render Simple View: ", path);
		phalcon_debug_print_r(&debug_message);
	}

	ZVAL_TRUE(&not_exists);

	views_dir = phalcon_read_property(getThis(), SL("_viewsDir"), PH_NOISY);

	PHALCON_CONCAT_VV(&views_dir_path, views_dir, path);

	/** 
	 * Load the template engines
	 */
	PHALCON_CALL_METHODW(&engines, getThis(), "_loadtemplateengines");

	/** 
	 * Views are rendered in each engine
	 */
	ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL(engines), idx, str_key, engine) {
		zval extension = {}, view_engine_path = {};
		if (str_key) {
			ZVAL_STR(&extension, str_key);
		} else {
			ZVAL_LONG(&extension, idx);
		}
		PHALCON_CONCAT_VV(&view_engine_path, &views_dir_path, &extension);

		if (phalcon_file_exists(&view_engine_path) == SUCCESS) {

			if (unlikely(PHALCON_GLOBAL(debug).enable_debug)) {
				PHALCON_CONCAT_SV(&debug_message, "--Found: ", &view_engine_path);
				phalcon_debug_print_r(&debug_message);
			}

			/** 
			 * Call beforeRenderView if there is a events manager available
			 */
			if (Z_TYPE_P(events_manager) == IS_OBJECT) {
				ZVAL_STRING(&event_name, "view:beforeRenderView");
				PHALCON_CALL_METHODW(&status, events_manager, "fire", &event_name, getThis(), &view_engine_path);
				if (PHALCON_IS_FALSE(&status)) {
					continue;
				}
			}
			
			PHALCON_CALL_METHODW(NULL, engine, "render", &view_engine_path, params, &PHALCON_GLOBAL(z_true));

			/** 
			 * Call afterRenderView if there is a events manager available
			 */
			ZVAL_FALSE(&not_exists);
			if (Z_TYPE_P(events_manager) == IS_OBJECT) {
				ZVAL_STRING(&event_name, "view:afterRenderView");
				PHALCON_CALL_METHODW(NULL, events_manager, "fire", &event_name, getThis());
			}

			break;
		} else if (unlikely(PHALCON_GLOBAL(debug).enable_debug)) {
			PHALCON_CONCAT_SV(&debug_message, "--Not Found: ", &view_engine_path);
			phalcon_debug_print_r(&debug_message);
		}
	} ZEND_HASH_FOREACH_END();

	/** 
	 * Always throw an exception if the view does not exist
	 */
	if (PHALCON_IS_TRUE(&not_exists)) {
		PHALCON_CONCAT_SVS(&exception_message, "View '", &views_dir_path, "' was not found in the views directory");
		PHALCON_THROW_EXCEPTION_ZVALW(phalcon_mvc_view_exception_ce, &exception_message);
		return;
	}

	/** 
	 * Call afterRender event
	 */
	if (Z_TYPE_P(events_manager) == IS_OBJECT) {
		ZVAL_STRING(&event_name, "view:afterRender");
		PHALCON_CALL_METHODW(NULL, events_manager, "fire", &event_name, getThis());
	}
}

/**
 * Renders a view
 *
 * @param string $path
 * @param array $params
 * @return string
 */
PHP_METHOD(Phalcon_Mvc_View_Simple, render){

	zval *path, *params = NULL, cache = {}, is_started = {}, key = {}, lifetime = {}, *cache_options, content = {}, *view_params, merged_params = {}, is_fresh = {};

	phalcon_fetch_params(0, 1, 1, &path, &params);

	if (!params) {
		params = &PHALCON_GLOBAL(z_null);
	}

	/** 
	 * Create/Get a cache
	 */
	PHALCON_CALL_METHODW(&cache, getThis(), "getcache");
	if (Z_TYPE(cache) == IS_OBJECT) {

		/** 
		 * Check if the cache is started, the first time a cache is started we start the
		 * cache
		 */
		PHALCON_CALL_METHODW(&is_started, &cache, "isstarted");
		if (PHALCON_IS_FALSE(&is_started)) {
			cache_options = phalcon_read_property(getThis(), SL("_cacheOptions"), PH_NOISY);

			/** 
			 * Check if the user has defined a different options to the default
			 */
			if (Z_TYPE_P(cache_options) == IS_ARRAY) { 
				if (phalcon_array_isset_str(cache_options, SL("key"))) {
					phalcon_array_fetch_str(&key, cache_options, SL("key"), PH_NOISY);
				}
				if (phalcon_array_isset_str(cache_options, SL("lifetime"))) {
					phalcon_array_fetch_str(&lifetime, cache_options, SL("lifetime"), PH_NOISY);
				}
			}

			/** 
			 * If a cache key is not set we create one using a md5
			 */
			if (Z_TYPE(key) == IS_NULL) {
				phalcon_md5(&key, path);
			}

			/** 
			 * We start the cache using the key set
			 */
			PHALCON_CALL_METHODW(&content, &cache, "start", &key, &lifetime);
			if (Z_TYPE(content) != IS_NULL) {
				phalcon_update_property_this(getThis(), SL("_content"), &content);
				RETURN_CTORW(&content);
			}
		}
	}

	phalcon_ob_start();

	view_params = phalcon_read_property(getThis(), SL("_viewParams"), PH_NOISY);

	/** 
	 * Merge parameters
	 */
	if (Z_TYPE_P(params) == IS_ARRAY) { 
		if (Z_TYPE_P(view_params) == IS_ARRAY) { 
			phalcon_fast_array_merge(&merged_params, view_params, params);
		} else {
			PHALCON_CPY_WRT(&merged_params, params);
		}
	} else {
		PHALCON_CPY_WRT(&merged_params, view_params);
	}

	/** 
	 * internalRender is also reused by partials
	 */
	PHALCON_CALL_METHODW(NULL, getThis(), "_internalrender", path, &merged_params);

	/** 
	 * Store the data in output into the cache
	 */
	if (Z_TYPE(cache) == IS_OBJECT) {
		PHALCON_CALL_METHODW(&is_started, &cache, "isstarted");
		if (PHALCON_IS_TRUE(&is_started)) {
			PHALCON_CALL_METHODW(&is_fresh, &cache, "isfresh");
			if (PHALCON_IS_TRUE(&is_fresh)) {
				PHALCON_CALL_METHODW(NULL, &cache, "save");
			} else {
				PHALCON_CALL_METHODW(NULL, &cache, "stop");
			}
		} else {
			PHALCON_CALL_METHODW(NULL, &cache, "stop");
		}
	}

	phalcon_ob_end_clean();

	phalcon_return_property(&content, getThis(), SL("_content"));

	RETURN_CTORW(&content);
}

/**
 * Renders a partial view
 *
 * <code>
 * 	//Show a partial inside another view
 * 	$this->partial('shared/footer');
 * </code>
 *
 * <code>
 * 	//Show a partial inside another view with parameters
 * 	$this->partial('shared/footer', array('content' => $html));
 * </code>
 *
 * @param string $partialPath
 * @param array $params
 */
PHP_METHOD(Phalcon_Mvc_View_Simple, partial){

	zval *partial_path, *params = NULL, *view_params, merged_params = {}, *content;

	phalcon_fetch_params(0, 1, 1, &partial_path, &params);

	if (!params) {
		params = &PHALCON_GLOBAL(z_null);
	}

	/** 
	 * Start ouput buffering
	 */
	phalcon_ob_start();

	/** 
	 * If the developer pass an array of variables we create a new virtual symbol table
	 */
	if (Z_TYPE_P(params) == IS_ARRAY) {
		view_params = phalcon_read_property(getThis(), SL("_viewParams"), PH_NOISY);

		/** 
		 * Merge or assign the new params as parameters
		 */
		if (Z_TYPE_P(view_params) == IS_ARRAY) {
			phalcon_fast_array_merge(&merged_params, view_params, params);
		} else {
			PHALCON_CPY_WRT(&merged_params, params);
		}
	} else {
		PHALCON_CPY_WRT(&merged_params, params);
	}

	/** 
	 * Call engine render, this checks in every registered engine for the partial
	 */
	PHALCON_CALL_METHODW(NULL, getThis(), "_internalrender", partial_path, &merged_params);

	/** 
	 * Now we need to restore the original view parameters
	 */
	if (view_params != NULL) {
		/** 
		 * Restore the original view params
		 */
		phalcon_update_property_this(getThis(), SL("_viewParams"), view_params);
	}

	phalcon_ob_end_clean();

	content = phalcon_read_property(getThis(), SL("_content"), PH_NOISY);

	/** 
	 * Content is output to the parent view
	 */
	zend_print_zval(content, 0);
}

/**
 * Sets the cache options
 *
 * @param array $options
 * @return Phalcon\Mvc\View\Simple
 */
PHP_METHOD(Phalcon_Mvc_View_Simple, setCacheOptions){

	zval *options;

	phalcon_fetch_params(0, 1, 0, &options);

	phalcon_update_property_this(getThis(), SL("_cacheOptions"), options);
	RETURN_THISW();
}

/**
 * Returns the cache options
 *
 * @return array
 */
PHP_METHOD(Phalcon_Mvc_View_Simple, getCacheOptions){


	RETURN_MEMBER(getThis(), "_cacheOptions");
}

/**
 * Create a Phalcon\Cache based on the internal cache options
 *
 * @return Phalcon\Cache\BackendInterface
 */
PHP_METHOD(Phalcon_Mvc_View_Simple, _createCache){

	zval *dependency_injector, cache_service = {}, *cache_options;

	dependency_injector = phalcon_read_property(getThis(), SL("_dependencyInjector"), PH_NOISY);
	if (Z_TYPE_P(dependency_injector) != IS_OBJECT) {
		PHALCON_THROW_EXCEPTION_STRW(phalcon_mvc_view_exception_ce, "A dependency injector container is required to obtain the view cache services");
		return;
	}

	cache_options = phalcon_read_property(getThis(), SL("_cacheOptions"), PH_NOISY);
	if (Z_TYPE_P(cache_options) != IS_ARRAY || !phalcon_array_isset_fetch_str(&cache_service, cache_options, SL("service"))) {
		ZVAL_STRING(&cache_service, "viewCache");
	}

	/** 
	 * The injected service must be an object
	 */
	PHALCON_CALL_METHODW(return_value, dependency_injector, "getshared", &cache_service);

	if (Z_TYPE_P(return_value) != IS_OBJECT) {
		PHALCON_THROW_EXCEPTION_STRW(phalcon_mvc_view_exception_ce, "The injected caching service is invalid");
		return;
	}

	PHALCON_VERIFY_INTERFACEW(return_value, phalcon_cache_backendinterface_ce);
}

/**
 * Returns the cache instance used to cache
 *
 * @return Phalcon\Cache\BackendInterface
 */
PHP_METHOD(Phalcon_Mvc_View_Simple, getCache){

	zval cache = {};

	phalcon_return_property(&cache, getThis(), SL("_cache"));
	if (zend_is_true(&cache)) {
		if (Z_TYPE(cache) != IS_OBJECT) {
			PHALCON_CALL_METHODW(&cache, getThis(), "_createcache");
			phalcon_update_property_this(getThis(), SL("_cache"), &cache);
		}
	}

	RETURN_CTORW(&cache);
}

/**
 * Cache the actual view render to certain level
 *
 *<code>
 *  $this->view->cache(array('key' => 'my-key', 'lifetime' => 86400));
 *</code>
 *
 * @param boolean|array $options
 * @return Phalcon\Mvc\View\Simple
 */
PHP_METHOD(Phalcon_Mvc_View_Simple, cache){

	zval *options = NULL;

	phalcon_fetch_params(0, 0, 1, &options);

	if (!options) {
		options = &PHALCON_GLOBAL(z_true);
	}

	if (Z_TYPE_P(options) == IS_ARRAY) { 
		phalcon_update_property_this(getThis(), SL("_cache"), &PHALCON_GLOBAL(z_true));
		phalcon_update_property_this(getThis(), SL("_cacheOptions"), options);
	} else {
		if (zend_is_true(options)) {
			phalcon_update_property_this(getThis(), SL("_cache"), &PHALCON_GLOBAL(z_true));
		} else {
			phalcon_update_property_this(getThis(), SL("_cache"), &PHALCON_GLOBAL(z_false));
		}
	}

	RETURN_THISW();
}

/**
 * Adds parameters to views (alias of setVar)
 *
 *<code>
 *	$this->view->setParamToView('products', $products);
 *</code>
 *
 * @param string $key
 * @param mixed $value
 * @return Phalcon\Mvc\View\Simple
 */
PHP_METHOD(Phalcon_Mvc_View_Simple, setParamToView){

	zval *key, *value;

	phalcon_fetch_params(0, 2, 0, &key, &value);

	phalcon_update_property_array(getThis(), SL("_viewParams"), key, value);
	RETURN_THISW();
}

/**
 * Set all the render params
 *
 *<code>
 *	$this->view->setVars(array('products' => $products));
 *</code>
 *
 * @param array $params
 * @param boolean $merge
 * @return Phalcon\Mvc\View\Simple
 */
PHP_METHOD(Phalcon_Mvc_View_Simple, setVars){

	zval *params, *merge = NULL, *view_params, merged_params = {};

	phalcon_fetch_params(0, 1, 1, &params, &merge);

	if (!merge) {
		merge = &PHALCON_GLOBAL(z_true);
	}

	if (Z_TYPE_P(params) != IS_ARRAY) { 
		PHALCON_THROW_EXCEPTION_STRW(phalcon_mvc_view_exception_ce, "The render parameters must be an array");
		return;
	}

	if (zend_is_true(merge)) {
		view_params = phalcon_read_property(getThis(), SL("_viewParams"), PH_NOISY);
		if (Z_TYPE_P(view_params) == IS_ARRAY) {
			phalcon_fast_array_merge(&merged_params, view_params, params);
		} else {
			PHALCON_CPY_WRT(&merged_params, params);
		}

		phalcon_update_property_this(getThis(), SL("_viewParams"), &merged_params);
	} else {
		phalcon_update_property_this(getThis(), SL("_viewParams"), params);
	}

	RETURN_THISW();
}

/**
 * Set a single view parameter
 *
 *<code>
 *	$this->view->setVar('products', $products);
 *</code>
 *
 * @param string $key
 * @param mixed $value
 * @return Phalcon\Mvc\View\Simple
 */
PHP_METHOD(Phalcon_Mvc_View_Simple, setVar){

	zval *key, *value;

	phalcon_fetch_params(0, 2, 0, &key, &value);

	phalcon_update_property_array(getThis(), SL("_viewParams"), key, value);
	RETURN_THISW();
}

/**
 * Returns a parameter previously set in the view
 *
 * @param string $key
 * @return mixed
 * @see __get()
 */
PHALCON_DOC_METHOD(Phalcon_Mvc_View_Simple, getVar);

/**
 * Returns parameters to views
 *
 * @return array
 */
PHP_METHOD(Phalcon_Mvc_View_Simple, getParamsToView){


	RETURN_MEMBER(getThis(), "_viewParams");
}

/**
 * Externally sets the view content
 *
 *<code>
 *	$this->view->setContent("<h1>hello</h1>");
 *</code>
 *
 * @param string $content
 * @return Phalcon\Mvc\View\Simple
 */
PHP_METHOD(Phalcon_Mvc_View_Simple, setContent){

	zval *content;

	phalcon_fetch_params(0, 1, 0, &content);

	if (Z_TYPE_P(content) != IS_STRING) {
		PHALCON_THROW_EXCEPTION_STRW(phalcon_mvc_view_exception_ce, "Content must be a string");
		return;
	}
	phalcon_update_property_this(getThis(), SL("_content"), content);

	RETURN_THISW();
}

/**
 * Returns cached ouput from another view stage
 *
 * @return string
 */
PHP_METHOD(Phalcon_Mvc_View_Simple, getContent){


	RETURN_MEMBER(getThis(), "_content");
}

/**
 * Returns the path of the view that is currently rendered
 *
 * @return string
 */
PHP_METHOD(Phalcon_Mvc_View_Simple, getActiveRenderPath){


	RETURN_MEMBER(getThis(), "_activeRenderPath");
}

/**
 * Magic method to pass variables to the views
 *
 *<code>
 *	$this->view->products = $products;
 *</code>
 *
 * @param string $key
 * @param mixed $value
 */
PHP_METHOD(Phalcon_Mvc_View_Simple, __set){

	zval *key, *value;

	phalcon_fetch_params(0, 2, 0, &key, &value);

	phalcon_update_property_array(getThis(), SL("_viewParams"), key, value);

}

/**
 * Magic method to retrieve a variable passed to the view
 *
 *<code>
 *	echo $this->view->products;
 *</code>
 *
 * @param string $key
 * @return mixed
 */
PHP_METHOD(Phalcon_Mvc_View_Simple, __get){

	zval *key, *params;

	phalcon_fetch_params(0, 1, 0, &key);

	params = phalcon_read_property(getThis(), SL("_viewParams"), PH_NOISY);
	if (!phalcon_array_isset_fetch(return_value, params, key)) {
		RETURN_NULL();
	}
}
