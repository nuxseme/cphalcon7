
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
  |          Nikolaos Dimopoulos <nikos@niden.net>                         |
  +------------------------------------------------------------------------+
*/

#include "di.h"
#include "diinterface.h"
#include "di/exception.h"
#include "di/injectionawareinterface.h"
#include "di/service.h"
#include "di/serviceinterface.h"
#include "di/factorydefault.h"
#include "events/managerinterface.h"

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/object.h"
#include "kernel/exception.h"
#include "kernel/fcall.h"
#include "kernel/array.h"
#include "kernel/concat.h"
#include "kernel/file.h"
#include "kernel/string.h"
#include "kernel/hash.h"

#include "internal/arginfo.h"

/**
 * Phalcon\DI
 *
 * Phalcon\DI is a component that implements Dependency Injection/Service Location
 * of services and it's itself a container for them.
 *
 * Since Phalcon is highly decoupled, Phalcon\DI is essential to integrate the different
 * components of the framework. The developer can also use this component to inject dependencies
 * and manage global instances of the different classes used in the application.
 *
 * Basically, this component implements the `Inversion of Control` pattern. Applying this,
 * the objects do not receive their dependencies using setters or constructors, but requesting
 * a service dependency injector. This reduces the overall complexity, since there is only one
 * way to get the required dependencies within a component.
 *
 * Additionally, this pattern increases testability in the code, thus making it less prone to errors.
 *
 *<code>
 * $di = new Phalcon\DI();
 *
 * //Using a string definition
 * $di->set('request', 'Phalcon\Http\Request', true);
 *
 * //Using an anonymous function
 * $di->set('request', function(){
 *	  return new Phalcon\Http\Request();
 * }, true);
 *
 * $request = $di->getRequest();
 *
 *</code>
 */
zend_class_entry *phalcon_di_ce;

PHP_METHOD(Phalcon_DI, __construct);
PHP_METHOD(Phalcon_DI, setEventsManager);
PHP_METHOD(Phalcon_DI, getEventsManager);
PHP_METHOD(Phalcon_DI, set);
PHP_METHOD(Phalcon_DI, setShared);
PHP_METHOD(Phalcon_DI, remove);
PHP_METHOD(Phalcon_DI, attempt);
PHP_METHOD(Phalcon_DI, getRaw);
PHP_METHOD(Phalcon_DI, setService);
PHP_METHOD(Phalcon_DI, getService);
PHP_METHOD(Phalcon_DI, get);
PHP_METHOD(Phalcon_DI, getShared);
PHP_METHOD(Phalcon_DI, has);
PHP_METHOD(Phalcon_DI, wasFreshInstance);
PHP_METHOD(Phalcon_DI, getServices);
PHP_METHOD(Phalcon_DI, __call);
PHP_METHOD(Phalcon_DI, setDefault);
PHP_METHOD(Phalcon_DI, getDefault);
PHP_METHOD(Phalcon_DI, reset);
PHP_METHOD(Phalcon_DI, __clone);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_di___call, 0, 0, 1)
	ZEND_ARG_INFO(0, method)
	ZEND_ARG_INFO(0, arguments)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_di_setshared, 0, 0, 2)
	ZEND_ARG_INFO(0, name)
	ZEND_ARG_INFO(0, definition)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_di_attempt, 0, 0, 2)
	ZEND_ARG_INFO(0, name)
	ZEND_ARG_INFO(0, definition)
	ZEND_ARG_INFO(0, shared)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_di_getraw, 0, 0, 1)
	ZEND_ARG_INFO(0, name)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_di_method_entry[] = {
	PHP_ME(Phalcon_DI, __construct, NULL, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_ME(Phalcon_DI, setEventsManager, NULL, ZEND_ACC_PROTECTED)
	PHP_ME(Phalcon_DI, getEventsManager, NULL, ZEND_ACC_PROTECTED)
	/* Phalcon\DiInterface*/
	PHP_ME(Phalcon_DI, set, arginfo_phalcon_diinterface_set, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_DI, remove, arginfo_phalcon_diinterface_remove, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_DI, getRaw, arginfo_phalcon_di_getraw, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_DI, getService, arginfo_phalcon_diinterface_getservice, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_DI, setService, arginfo_phalcon_diinterface_setservice, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_DI, get, arginfo_phalcon_diinterface_get, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_DI, getShared, arginfo_phalcon_diinterface_getshared, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_DI, has, arginfo_phalcon_diinterface_has, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_DI, wasFreshInstance, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_DI, getServices, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_DI, setDefault, arginfo_phalcon_diinterface_setdefault, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_DI, getDefault, NULL, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_DI, reset, NULL, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)

	/* Convenience methods */
	PHP_ME(Phalcon_DI, attempt, arginfo_phalcon_di_attempt, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_DI, setShared, arginfo_phalcon_di_setshared, ZEND_ACC_PUBLIC)
	PHP_MALIAS(Phalcon_DI, setRaw, setService, arginfo_phalcon_diinterface_setservice, ZEND_ACC_PUBLIC | ZEND_ACC_DEPRECATED)

	/* Syntactic sugar */
	PHP_MALIAS(Phalcon_DI, offsetExists, has, arginfo_arrayaccess_offsetexists, ZEND_ACC_PUBLIC)
	PHP_MALIAS(Phalcon_DI, offsetSet, setShared, arginfo_arrayaccess_offsetset, ZEND_ACC_PUBLIC)
	PHP_MALIAS(Phalcon_DI, offsetGet, getShared, arginfo_arrayaccess_offsetget, ZEND_ACC_PUBLIC)
	PHP_MALIAS(Phalcon_DI, offsetUnset, remove, arginfo_arrayaccess_offsetunset, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_DI, __call, arginfo_phalcon_di___call, ZEND_ACC_PUBLIC)

	/* Misc */
	PHP_ME(Phalcon_DI, __clone, NULL, ZEND_ACC_PUBLIC)

	PHP_MALIAS(Phalcon_DI, __set, set, arginfo___set, ZEND_ACC_PUBLIC)
	PHP_MALIAS(Phalcon_DI, __get, get, arginfo___get, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\DI initializer
 */
PHALCON_INIT_CLASS(Phalcon_DI){

	PHALCON_REGISTER_CLASS(Phalcon, DI, di, phalcon_di_method_entry, 0);

	zend_declare_property_null(phalcon_di_ce, SL("_services"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_di_ce, SL("_sharedInstances"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_di_ce, SL("_freshInstance"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_di_ce, SL("_default"), ZEND_ACC_PROTECTED|ZEND_ACC_STATIC);
	zend_declare_property_null(phalcon_di_ce, SL("_eventsManager"), ZEND_ACC_PROTECTED);

	zend_class_implements(phalcon_di_ce, 1, phalcon_diinterface_ce);

	return SUCCESS;
}

/**
 * Phalcon\DI constructor
 *
 */
PHP_METHOD(Phalcon_DI, __construct){

	zval *default_di;

	default_di = phalcon_read_static_property_ce(phalcon_di_ce, SL("_default"));
	if (Z_TYPE_P(default_di) == IS_NULL) {
		phalcon_update_static_property_ce(phalcon_di_ce, SL("_default"), getThis());
	}
}

/**
 * Sets a custom events manager
 *
 * @param Phalcon\Events\ManagerInterface $eventsManager
 */
PHP_METHOD(Phalcon_DI, setEventsManager){

	zval *events_manager;

	phalcon_fetch_params(0, 1, 0, &events_manager);
	PHALCON_VERIFY_INTERFACE_EX(events_manager, phalcon_events_managerinterface_ce, phalcon_di_exception_ce, 0);

	phalcon_update_property_this(getThis(), SL("_eventsManager"), events_manager);
}

/**
 * Returns the custom events manager
 *
 * @return Phalcon\Events\ManagerInterface
 */
PHP_METHOD(Phalcon_DI, getEventsManager){

	RETURN_MEMBER(getThis(), "_eventsManager");
}

/**
 * Registers a service in the services container
 *
 * @param string $name
 * @param mixed $definition
 * @param boolean $shared
 * @return Phalcon\DI\ServiceInterface
 */
PHP_METHOD(Phalcon_DI, set) {

	zval *name, *definition, *shared = NULL;

	phalcon_fetch_params(0, 2, 1, &name, &definition, &shared);
	PHALCON_ENSURE_IS_STRING(name);
	
	if (!shared) {
		shared = &PHALCON_GLOBAL(z_false);
	}

	object_init_ex(return_value, phalcon_di_service_ce);
	PHALCON_CALL_METHODW(NULL, return_value, "__construct", name, definition, shared);

	phalcon_update_property_array(getThis(), SL("_services"), name, return_value);
}

/**
 * Registers an "always shared" service in the services container
 *
 * @param string $name
 * @param mixed $definition
 * @return Phalcon\DI\ServiceInterface
 */
PHP_METHOD(Phalcon_DI, setShared){

	zval *name, *definition;

	phalcon_fetch_params(0, 2, 0, &name, &definition);
	PHALCON_ENSURE_IS_STRING(name);

	object_init_ex(return_value, phalcon_di_service_ce);
	PHALCON_CALL_METHODW(NULL, return_value, "__construct", name, definition, &PHALCON_GLOBAL(z_true));

	phalcon_update_property_array(getThis(), SL("_services"), name, return_value);
}

/**
 * Removes a service in the services container
 *
 * @param string $name
 */
PHP_METHOD(Phalcon_DI, remove){

	zval *name;

	phalcon_fetch_params(0, 1, 0, &name);
	PHALCON_ENSURE_IS_STRING(name);

	phalcon_unset_property_array(getThis(), SL("_services"), name);
}

/**
 * Attempts to register a service in the services container
 * Only is successful if a service hasn't been registered previously
 * with the same name
 *
 * @param string $name
 * @param mixed $definition
 * @param boolean $shared
 * @return Phalcon\DI\ServiceInterface
 */
PHP_METHOD(Phalcon_DI, attempt){

	zval *name, *definition, *shared = NULL;

	phalcon_fetch_params(0, 2, 1, &name, &definition, &shared);
	PHALCON_ENSURE_IS_STRING(name);

	if (!phalcon_isset_property_array(getThis(), SL("_services"), name)) {
		if (!shared) {
			shared = &PHALCON_GLOBAL(z_false);
		}

		object_init_ex(return_value, phalcon_di_service_ce);
		PHALCON_CALL_METHODW(NULL, return_value, "__construct", name, definition, shared);
	
		phalcon_update_property_array(getThis(), SL("_services"), name, return_value);
	}
}

/**
 * Sets a service using a raw Phalcon\DI\Service definition
 *
 * @param string|Phalcon\DI\ServiceInterface $raw_definition_or_name
 * @param Phalcon\DI\ServiceInterface $rawDefinition
 * @return Phalcon\DI\ServiceInterface
 */
PHP_METHOD(Phalcon_DI, setService)
{
	zval *name, *raw_definition;

	phalcon_fetch_params(0, 1, 1, &name, &raw_definition);

	phalcon_update_property_array(getThis(), SL("_services"), name, raw_definition);

	RETURN_CTORW(raw_definition);
}

/**
 * Returns a service definition without resolving
 *
 * @param string $name
 * @return mixed
 */
PHP_METHOD(Phalcon_DI, getRaw){

	zval *name, *service;

	phalcon_fetch_params(0, 1, 0, &name);
	PHALCON_ENSURE_IS_STRING(name);

	if (phalcon_isset_property_array(getThis(), SL("_services"), name)) {
		service = phalcon_read_property_array(getThis(), SL("_services"), name);
		PHALCON_RETURN_CALL_METHODW(service, "getdefinition");
		return;
	}

	zend_throw_exception_ex(phalcon_di_exception_ce, 0, "Service '%s' was not found in the dependency injection container", Z_STRVAL_P(name));
}

/**
 * Returns a Phalcon\DI\Service instance
 *
 * @param string $name
 * @return Phalcon\DI\ServiceInterface
 */
PHP_METHOD(Phalcon_DI, getService){

	zval *name, *service;

	phalcon_fetch_params(0, 1, 0, &name);
	PHALCON_ENSURE_IS_STRING(name);
	
	if (phalcon_isset_property_array(getThis(), SL("_services"), name)) {
		service = phalcon_read_property_array(getThis(), SL("_services"), name);
		RETURN_ZVAL(service, 1, 0);
	}

	zend_throw_exception_ex(phalcon_di_exception_ce, 0, "Service '%s' was not found in the dependency injection container", Z_STRVAL_P(name));
}

/**
 * Resolves the service based on its configuration
 *
 * @param string $name
 * @param array $parameters
 * @return mixed
 */
PHP_METHOD(Phalcon_DI, get){

	zval *name, *parameters = NULL, *noerror = NULL, events_manager = {}, event_name = {}, event_data = {}, service = {};
	zend_class_entry *ce;

	phalcon_fetch_params(0, 1, 2, &name, &parameters, &noerror);
	PHALCON_ENSURE_IS_STRING(name);

	if (!parameters) {
		parameters = &PHALCON_GLOBAL(z_null);
	}

	if (!noerror) {
		noerror = &PHALCON_GLOBAL(z_false);
	}

	phalcon_return_property(&events_manager, getThis(), SL("_eventsManager"));
	if (Z_TYPE(events_manager) == IS_OBJECT) {
		PHALCON_STR(&event_name, "di:beforeServiceResolve");

		array_init(&event_data);
		phalcon_array_update_str(&event_data, SL("name"), name, PH_COPY);
		phalcon_array_update_str(&event_data, SL("parameters"), parameters, PH_COPY);

		PHALCON_CALL_METHODW(NULL, &events_manager, "fire", &event_name, getThis(), &event_data);
		PHALCON_PTR_DTOR(&event_data);
	}

	if (phalcon_property_array_isset_fetch(&service, getThis(), SL("_services"), name)) {
		PHALCON_CALL_METHODW(return_value, &service, "resolve", parameters, getThis());
		ce = (Z_TYPE_P(return_value) == IS_OBJECT) ? Z_OBJCE_P(return_value) : NULL;
		PHALCON_PTR_DTOR(&service);
	} else {
		/* The DI also acts as builder for any class even if it isn't defined in the DI */
		if ((ce = phalcon_class_exists_ex(name, 1)) != NULL) {
			if (FAILURE == phalcon_create_instance_params_ce(return_value, ce, parameters)) {
				return;
			}
		} else {
			if(!zend_is_true(noerror)) {
				zend_throw_exception_ex(phalcon_di_exception_ce, 0, "Service '%s' was not found in the dependency injection container", Z_STRVAL_P(name));
			}
			RETURN_NULL();
		}
	}

	/* Pass the DI itself if the instance implements Phalcon\DI\InjectionAwareInterface */
	if (ce && instanceof_function_ex(ce, phalcon_di_injectionawareinterface_ce, 1)) {
		PHALCON_CALL_METHODW(NULL, return_value, "setdi", getThis());
	}

	if (Z_TYPE(events_manager) == IS_OBJECT) {
		PHALCON_STR(&event_name, "di:afterServiceResolve");

		array_init(&event_data);
		phalcon_array_update_str(&event_data, SL("name"), name, PH_COPY);
		phalcon_array_update_str(&event_data, SL("parameters"), parameters, PH_COPY);
		phalcon_array_update_str(&event_data, SL("instance"), return_value, PH_COPY);

		PHALCON_CALL_METHODW(NULL, &events_manager, "fire", &event_name, getThis(), &event_data);
		PHALCON_PTR_DTOR(&event_data);
	}
	PHALCON_PTR_DTOR(&event_name);
	PHALCON_PTR_DTOR(&events_manager);
}

/**
 * Resolves a service, the resolved service is stored in the DI, subsequent requests for this service will return the same instance
 *
 * @param string $name
 * @param array $parameters
 * @return mixed
 */
PHP_METHOD(Phalcon_DI, getShared){

	zval *name, *parameters = NULL, *noerror = NULL;

	phalcon_fetch_params(0, 1, 2, &name, &parameters, &noerror);
	PHALCON_ENSURE_IS_STRING(name);

	if (!parameters) {
		parameters = &PHALCON_GLOBAL(z_null);
	}

	if (!noerror) {
		noerror = &PHALCON_GLOBAL(z_null);
	}

	if (phalcon_property_array_isset_fetch(return_value, getThis(), SL("_sharedInstances"), name)) {
		phalcon_update_property_bool(getThis(), SL("_freshInstance"), 0);
	} else {
		PHALCON_CALL_SELFW(return_value, "get", name, parameters, noerror);
		if (zend_is_true(return_value)) {
			phalcon_update_property_bool(getThis(), SL("_freshInstance"), 1);
			phalcon_update_property_array(getThis(), SL("_sharedInstances"), name, return_value);
		}
	}
}

/**
 * Check whether the DI contains a service by a name
 *
 * @param string $name
 * @return boolean
 */
PHP_METHOD(Phalcon_DI, has){

	zval *name;

	phalcon_fetch_params(0, 1, 0, &name);
	PHALCON_ENSURE_IS_STRING(name);
	
	if (phalcon_isset_property_array(getThis(), SL("_services"), name)) {
		RETURN_TRUE;
	}

	RETURN_FALSE;
}

/**
 * Check whether the last service obtained via getShared produced a fresh instance or an existing one
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_DI, wasFreshInstance){


	RETURN_MEMBER(getThis(), "_freshInstance");
}

/**
 * Return the services registered in the DI
 *
 * @return Phalcon\DI\Service[]
 */
PHP_METHOD(Phalcon_DI, getServices){

	RETURN_MEMBER(getThis(), "_services");
}

/**
 * Check if a service is registered using the array syntax.
 * Alias for Phalcon\Di::has()
 *
 * @param string $name
 * @return boolean
 */
PHALCON_DOC_METHOD(Phalcon_DI, offsetExists);

/**
 * Allows to register a shared service using the array syntax.
 * Alias for Phalcon\Di::setShared()
 *
 *<code>
 *	$di['request'] = new Phalcon\Http\Request();
 *</code>
 *
 * @param string $name
 * @param mixed $definition
 */
PHALCON_DOC_METHOD(Phalcon_DI, offsetSet);

/**
 * Allows to obtain a shared service using the array syntax.
 * Alias for Phalcon\Di::getShared()
 *
 *<code>
 *	var_dump($di['request']);
 *</code>
 *
 * @param string $name
 * @return mixed
 */
PHALCON_DOC_METHOD(Phalcon_DI, offsetGet);

/**
 * Removes a service from the services container using the array syntax.
 * Alias for Phalcon\Di::remove()
 *
 * @param string $name
 */
PHALCON_DOC_METHOD(Phalcon_DI, offsetUnset);

/**
 * Magic method to get or set services using setters/getters
 *
 * @param string $method
 * @param array $arguments
 * @return mixed
 */
PHP_METHOD(Phalcon_DI, __call){

	zval *method, *arguments = NULL, possible_service = {}, name = {}, definition = {};

	phalcon_fetch_params(0, 1, 1, &method, &arguments);
	PHALCON_ENSURE_IS_STRING(method);

	if (!arguments) {
		arguments = &PHALCON_GLOBAL(z_null);
	}

	phalcon_substr(&possible_service, method, 3, 0);
	phalcon_lcfirst(&name, &possible_service);

	/**
	 * If the magic method starts with "get" we try to get a service with that name
	 */
	if (phalcon_start_with_str(method, SL("get"))) {
		if (phalcon_isset_property_array(getThis(), SL("_services"), &name)) {
			PHALCON_RETURN_CALL_SELFW("get", &name, arguments);
			return;
		}
	}

	/**
	 * If the magic method starts with "set" we try to set a service using that name
	 */
	if (phalcon_start_with_str(method, SL("set"))) {
		if (phalcon_array_isset_fetch_long(&definition, arguments, 0)) {
			PHALCON_CALL_SELFW(NULL, "set", &name, &definition);
			return;
		}
	}

	/**
	 * The method doesn't start with set/get throw an exception
	 */
	zend_throw_exception_ex(phalcon_di_exception_ce, 0, "Call to undefined method or service '%s'", Z_STRVAL_P(method));
}

/**
 * Set a default dependency injection container to be obtained into static methods
 *
 * @param Phalcon\DiInterface $dependencyInjector
 */
PHP_METHOD(Phalcon_DI, setDefault){

	zval *dependency_injector;

	phalcon_fetch_params(0, 1, 0, &dependency_injector);
	PHALCON_VERIFY_INTERFACE_EX(dependency_injector, phalcon_diinterface_ce, phalcon_di_exception_ce, 0);

	phalcon_update_static_property_ce(phalcon_di_ce, SL("_default"), dependency_injector);
}

/**
 * Return the lastest DI created
 *
 * @return Phalcon\DiInterface
 */
PHP_METHOD(Phalcon_DI, getDefault){

	phalcon_return_static_property_ce(return_value, phalcon_di_ce, SL("_default"));
	if (Z_TYPE_P(return_value) != IS_OBJECT) {
		object_init_ex(return_value, phalcon_di_factorydefault_ce);
		PHALCON_CALL_METHODW(NULL, return_value, "__construct");
	}
}

/**
 * Resets the internal default DI
 */
PHP_METHOD(Phalcon_DI, reset){

	zend_update_static_property_null(phalcon_di_ce, SL("_default"));
}

PHP_METHOD(Phalcon_DI, __clone) {
}
