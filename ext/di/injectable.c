
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

#include "di/injectable.h"
#include "di/exception.h"
#include "di/injectionawareinterface.h"
#include "diinterface.h"
#include "di.h"
#include "events/eventsawareinterface.h"
#include "events/managerinterface.h"
#include "diinterface.h"

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/exception.h"
#include "kernel/object.h"
#include "kernel/fcall.h"
#include "kernel/concat.h"
#include "kernel/array.h"
#include "kernel/operators.h"
#include "kernel/string.h"

#include "internal/arginfo.h"
#include "interned-strings.h"

/**
 * Phalcon\DI\Injectable
 *
 * This class allows to access services in the services container by just only accessing a public property
 * with the same name of a registered service
 */
zend_class_entry *phalcon_di_injectable_ce;

PHP_METHOD(Phalcon_DI_Injectable, setDI);
PHP_METHOD(Phalcon_DI_Injectable, getDI);
PHP_METHOD(Phalcon_DI_Injectable, setEventsManager);
PHP_METHOD(Phalcon_DI_Injectable, getEventsManager);
PHP_METHOD(Phalcon_DI_Injectable, fireEvent);
PHP_METHOD(Phalcon_DI_Injectable, fireEventCancel);
PHP_METHOD(Phalcon_DI_Injectable, getResolveService);
PHP_METHOD(Phalcon_DI_Injectable, __get);

static const zend_function_entry phalcon_di_injectable_method_entry[] = {
	PHP_ME(Phalcon_DI_Injectable, setDI, arginfo_phalcon_di_injectionawareinterface_setdi, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_DI_Injectable, getDI, arginfo_phalcon_di_injectionawareinterface_getdi, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_DI_Injectable, setEventsManager, arginfo_phalcon_events_eventsawareinterface_seteventsmanager, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_DI_Injectable, getEventsManager, arginfo_phalcon_events_eventsawareinterface_geteventsmanager, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_DI_Injectable, fireEvent, arginfo_phalcon_di_injectable_fireevent, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_DI_Injectable, fireEventCancel, arginfo_phalcon_di_injectable_fireeventcancel, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_DI_Injectable, getResolveService, arginfo_phalcon_di_injectable_getresolveservice, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_DI_Injectable, __get, arginfo___get, ZEND_ACC_PUBLIC)
	PHP_FE_END
};


/**
 * Phalcon\DI\Injectable initializer
 */
PHALCON_INIT_CLASS(Phalcon_DI_Injectable){

	PHALCON_REGISTER_CLASS(Phalcon\\DI, Injectable, di_injectable, phalcon_di_injectable_method_entry, ZEND_ACC_EXPLICIT_ABSTRACT_CLASS);

	zend_declare_property_null(phalcon_di_injectable_ce, SL("_dependencyInjector"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_di_injectable_ce, SL("_eventsManager"), ZEND_ACC_PROTECTED);

	zend_class_implements(phalcon_di_injectable_ce, 2, phalcon_di_injectionawareinterface_ce, phalcon_events_eventsawareinterface_ce);

	return SUCCESS;
}

/**
 * Sets the dependency injector
 *
 * @param Phalcon\DiInterface $dependencyInjector
 * @throw Phalcon\Di\Exception
 */
PHP_METHOD(Phalcon_DI_Injectable, setDI){

	zval *dependency_injector;

	phalcon_fetch_params(0, 1, 0, &dependency_injector);

	PHALCON_VERIFY_INTERFACE_OR_NULL_EX(dependency_injector, phalcon_diinterface_ce, phalcon_di_exception_ce, 0);
	phalcon_update_property_this(getThis(), SL("_dependencyInjector"), dependency_injector);

	RETURN_THISW();
}

/**
 * Returns the internal dependency injector
 *
 * @return Phalcon\DiInterface
 */
PHP_METHOD(Phalcon_DI_Injectable, getDI)
{
	zval *error = NULL, dependency_injector = {};

	phalcon_fetch_params(0, 0, 1, &error);

	if (!error) {
		error = &PHALCON_GLOBAL(z_false);
	}

	phalcon_return_property(&dependency_injector, getThis(), SL("_dependencyInjector"));
	if (Z_TYPE(dependency_injector) != IS_OBJECT) {
		PHALCON_CALL_CE_STATICW(&dependency_injector, phalcon_di_ce, "getdefault");
		phalcon_update_property_this(getThis(), SL("_dependencyInjector"), &dependency_injector);
	}

	if (Z_TYPE(dependency_injector) != IS_OBJECT && zend_is_true(error)) {
		PHALCON_THROW_EXCEPTION_STRW(phalcon_di_exception_ce, "A dependency injection container is not object");
		return;
	}
	RETURN_CTORW(&dependency_injector);
}

/**
 * Sets the event manager
 *
 * @param Phalcon\Events\ManagerInterface $eventsManager
 */
PHP_METHOD(Phalcon_DI_Injectable, setEventsManager)
{
	zval *events_manager;

	phalcon_fetch_params(0, 1, 0, &events_manager);
	PHALCON_VERIFY_INTERFACE_OR_NULL_EX(events_manager, phalcon_events_managerinterface_ce, phalcon_di_exception_ce, 0);

	phalcon_update_property_this(getThis(), SL("_eventsManager"), events_manager);

	RETURN_THISW();
}

/**
 * Returns the internal event manager
 *
 * @return Phalcon\Events\ManagerInterface
 */
PHP_METHOD(Phalcon_DI_Injectable, getEventsManager){


	RETURN_MEMBER(getThis(), "_eventsManager");
}

/**
 * Fires an event, implicitly calls behaviors and listeners in the events manager are notified
 *
 * @param string $eventName
 * @return boolean
 */
PHP_METHOD(Phalcon_DI_Injectable, fireEvent){

	zval *eventname, *data = NULL, *cancelable = NULL, events_manager = {}, lower = {}, event_parts = {}, name = {}, status = {};

	phalcon_fetch_params(0, 1, 2, &eventname, &data, &cancelable);
	PHALCON_ENSURE_IS_STRING(eventname);

	if (!data) {
		data = &PHALCON_GLOBAL(z_null);
	}

	if (!cancelable) {
		cancelable = &PHALCON_GLOBAL(z_true);
	}

	phalcon_fast_strtolower(&lower, eventname);

	if (phalcon_memnstr_str(&lower, SL(":"))) {
		phalcon_fast_explode_str(&event_parts, SL(":"), &lower);
		phalcon_array_fetch_long(&name, &event_parts, 1, PH_NOISY);
		PHALCON_PTR_DTOR(&event_parts);
	} else {
		PHALCON_CPY_WRT(&name, &lower);
	}

	/**
	 * Check if there is a method with the same name of the event
	 */
	if (phalcon_method_exists(getThis(), &name) == SUCCESS) {
		PHALCON_CALL_METHODW(NULL, getThis(), Z_STRVAL(name), data);
	}

	PHALCON_PTR_DTOR(&name);
	PHALCON_PTR_DTOR(&lower);

	phalcon_return_property(&events_manager, getThis(), SL("_eventsManager"));

	if (Z_TYPE(events_manager) != IS_NULL) {
		PHALCON_VERIFY_INTERFACE_EX(&events_manager, phalcon_events_managerinterface_ce, phalcon_di_exception_ce, 0);

		/**
		 * Send a notification to the events manager
		 */
		PHALCON_CALL_METHODW(&status, &events_manager, "fire", eventname, getThis(), data, cancelable);
		if (PHALCON_IS_FALSE(&status)) {
			PHALCON_PTR_DTOR(&events_manager);
			RETURN_FALSE;
		}
		PHALCON_PTR_DTOR(&status);
		PHALCON_PTR_DTOR(&events_manager);
	}

	RETURN_TRUE;
}

/**
 * Fires an event, implicitly calls behaviors and listeners in the events manager are notified
 * This method stops if one of the callbacks/listeners returns boolean false
 *
 * @param string $eventName
 * @return boolean
 */
PHP_METHOD(Phalcon_DI_Injectable, fireEventCancel){

	zval *eventname, *data = NULL, *cancelable = NULL, status = {}, events_manager = {}, lower = {}, event_parts = {}, name = {};

	phalcon_fetch_params(0, 1, 2, &eventname, &data, &cancelable);
	PHALCON_ENSURE_IS_STRING(eventname);

	if (!data) {
		data = &PHALCON_GLOBAL(z_null);
	}

	if (!cancelable) {
		cancelable = &PHALCON_GLOBAL(z_true);
	}

	phalcon_fast_strtolower(&lower, eventname);

	if (phalcon_memnstr_str(&lower, SL(":"))) {
		phalcon_fast_explode_str(&event_parts, SL(":"), &lower);
		phalcon_array_fetch_long(&name, &event_parts, 1, PH_NOISY);
		PHALCON_PTR_DTOR(&event_parts);
	} else {
		PHALCON_CPY_WRT(&name, &lower);
	}

	/**
	 * Check if there is a method with the same name of the event
	 */
	if (phalcon_method_exists(getThis(), &name) == SUCCESS) {
		PHALCON_CALL_METHODW(&status, getThis(), Z_STRVAL(name), data);
		if (PHALCON_IS_FALSE(&status)) {
			RETURN_FALSE;
		}
	}

	PHALCON_PTR_DTOR(&name);
	PHALCON_PTR_DTOR(&lower);

	phalcon_return_property(&events_manager, getThis(), SL("_eventsManager"));
	if (Z_TYPE(events_manager) != IS_NULL) {
		PHALCON_VERIFY_INTERFACE_EX(&events_manager, phalcon_events_managerinterface_ce, phalcon_di_exception_ce, 0);

		/**
		 * Send a notification to the events manager
		 */
		PHALCON_CALL_METHODW(&status, &events_manager, "fire", eventname, getThis(), data, cancelable);
		if (PHALCON_IS_FALSE(&status)) {
			PHALCON_PTR_DTOR(&events_manager);
			RETURN_FALSE;
		}
		PHALCON_PTR_DTOR(&events_manager);
	}

	PHALCON_PTR_DTOR(&status);

	RETURN_TRUE;
}

/**
 * Magic method __get
 *
 * @param string $propertyName
 */
PHP_METHOD(Phalcon_DI_Injectable, getResolveService){

	zval *name, *args = NULL, *noerror = NULL, *noshared = NULL, dependency_injector = {};

	phalcon_fetch_params(0, 1, 3, &name, &args, &noerror, &noshared);

	if (!args) {
		args = &PHALCON_GLOBAL(z_null);
	}

	if (!noerror) {
		noerror = &PHALCON_GLOBAL(z_false);
	}

	if (!noshared) {
		noshared = &PHALCON_GLOBAL(z_false);
	}

	ZVAL_NULL(return_value);

	PHALCON_CALL_METHODW(&dependency_injector, getThis(), "getdi", noerror);
	if (Z_TYPE(dependency_injector) == IS_OBJECT) {
		if (zend_is_true(noshared)) {
			PHALCON_RETURN_CALL_METHODW(&dependency_injector, "get", name, args, noerror);
		} else {
			PHALCON_RETURN_CALL_METHODW(&dependency_injector, "getshared", name, args, noerror);
		}
	}
}

/**
 * Magic method __get
 *
 * @param string $propertyName
 */
PHP_METHOD(Phalcon_DI_Injectable, __get){

	zval *property_name, dependency_injector = {}, has_service = {}, service = {}, class_name = {}, arguments = {}, result = {};

	phalcon_fetch_params(0, 1, 0, &property_name);
	PHALCON_ENSURE_IS_STRING(property_name);

	PHALCON_CALL_METHODW(&dependency_injector, getThis(), "getdi");
	PHALCON_CALL_METHODW(&has_service, &dependency_injector, "has", property_name);

	if (zend_is_true(&has_service)) {
		PHALCON_CALL_METHODW(&result, &dependency_injector, "getshared", property_name);
		phalcon_update_property_zval(getThis(), Z_STRVAL_P(property_name), Z_STRLEN_P(property_name), &result);
		RETURN_CTORW(&result);
	}

	if (Z_STRLEN_P(property_name) == sizeof("di")-1 && !memcmp(Z_STRVAL_P(property_name), "di", sizeof("di")-1)) {
		zend_update_property(phalcon_di_injectable_ce, getThis(), SL("di"), &dependency_injector);
		RETURN_CTORW(&dependency_injector);
	}

	/**
	 * Accessing the persistent property will create a session bag in any class
	 */
	if (Z_STRLEN_P(property_name) == sizeof("persistent")-1 && !memcmp(Z_STRVAL_P(property_name), "persistent", sizeof("persistent")-1)) {
		PHALCON_STR(&class_name, Z_OBJCE_P(getThis())->name->val);

		array_init_size(&arguments, 1);
		add_next_index_zval(&arguments, &class_name);

		PHALCON_STR(&service, "sessionBag");
		PHALCON_CALL_METHODW(&result, &dependency_injector, "get", &service, &arguments);
		PHALCON_PTR_DTOR(&service);
		PHALCON_PTR_DTOR(&arguments);
	
		zend_update_property(phalcon_di_injectable_ce, getThis(), SL("persistent"), &result);
		RETURN_CTORW(&result);
	}

	/**
	 * A notice is shown if the property is not defined or is not a valid service
	 */
	php_error_docref(NULL, E_WARNING, "Access to undefined property %s::%s", Z_OBJCE_P(getThis())->name->val, Z_STRVAL_P(property_name));
	RETURN_NULL();
}
