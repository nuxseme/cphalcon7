
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
  |          Rack Lin <racklin@gmail.com>                                  |
  +------------------------------------------------------------------------+
*/

#include "cli/router.h"
#include "cli/router/exception.h"
#include "diinterface.h"
#include "di/injectable.h"

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/exception.h"
#include "kernel/array.h"
#include "kernel/string.h"
#include "kernel/object.h"

/**
 * Phalcon\CLI\Router
 *
 * <p>Phalcon\CLI\Router is the standard framework router. Routing is the
 * process of taking a command-line arguments and
 * decomposing it into parameters to determine which module, task, and
 * action of that task should receive the request</p>
 *
 *<code>
 *	$router = new Phalcon\CLI\Router();
 *	$router->handle(array(
 *		'module' => 'main',
 *		'task' => 'videos',
 *		'action' => 'process'
 *	));
 *	echo $router->getTaskName();
 *</code>
 *
 */
zend_class_entry *phalcon_cli_router_ce;

PHP_METHOD(Phalcon_CLI_Router, __construct);
PHP_METHOD(Phalcon_CLI_Router, setDefaultModule);
PHP_METHOD(Phalcon_CLI_Router, setDefaultNamespace);
PHP_METHOD(Phalcon_CLI_Router, setDefaultTask);
PHP_METHOD(Phalcon_CLI_Router, setDefaultAction);
PHP_METHOD(Phalcon_CLI_Router, handle);
PHP_METHOD(Phalcon_CLI_Router, getModuleName);
PHP_METHOD(Phalcon_CLI_Router, getNamespaceName);
PHP_METHOD(Phalcon_CLI_Router, getTaskName);
PHP_METHOD(Phalcon_CLI_Router, getActionName);
PHP_METHOD(Phalcon_CLI_Router, getParams);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_cli_router_setdi, 0, 0, 1)
	ZEND_ARG_INFO(0, dependencyInjector)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_cli_router_setdefaultmodule, 0, 0, 1)
	ZEND_ARG_INFO(0, moduleName)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_cli_router_setdefaultnamespace, 0, 0, 1)
	ZEND_ARG_INFO(0, namespaceName)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_cli_router_setdefaulttask, 0, 0, 1)
	ZEND_ARG_INFO(0, taskName)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_cli_router_setdefaultaction, 0, 0, 1)
	ZEND_ARG_INFO(0, actionName)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_cli_router_handle, 0, 0, 0)
	ZEND_ARG_INFO(0, arguments)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_cli_router_method_entry[] = {
	PHP_ME(Phalcon_CLI_Router, __construct, NULL, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_ME(Phalcon_CLI_Router, setDefaultModule, arginfo_phalcon_cli_router_setdefaultmodule, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_CLI_Router, setDefaultNamespace, arginfo_phalcon_cli_router_setdefaultnamespace, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_CLI_Router, setDefaultTask, arginfo_phalcon_cli_router_setdefaulttask, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_CLI_Router, setDefaultAction, arginfo_phalcon_cli_router_setdefaultaction, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_CLI_Router, handle, arginfo_phalcon_cli_router_handle, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_CLI_Router, getNamespaceName, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_CLI_Router, getModuleName, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_CLI_Router, getTaskName, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_CLI_Router, getActionName, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_CLI_Router, getParams, NULL, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\CLI\Router initializer
 */
PHALCON_INIT_CLASS(Phalcon_CLI_Router){

	PHALCON_REGISTER_CLASS_EX(Phalcon\\CLI, Router, cli_router, phalcon_di_injectable_ce, phalcon_cli_router_method_entry, 0);

	zend_declare_property_null(phalcon_cli_router_ce, SL("_namespace"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_cli_router_ce, SL("_module"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_cli_router_ce, SL("_task"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_cli_router_ce, SL("_action"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_cli_router_ce, SL("_params"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_cli_router_ce, SL("_defaultModule"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_cli_router_ce, SL("_defaultNamespace"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_cli_router_ce, SL("_defaultTask"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_cli_router_ce, SL("_defaultAction"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_cli_router_ce, SL("_defaultParams"), ZEND_ACC_PROTECTED);

	return SUCCESS;
}

/**
 * Phalcon\CLI\Router constructor
 */
PHP_METHOD(Phalcon_CLI_Router, __construct){

	phalcon_update_property_empty_array(getThis(), SL("_params"));
	phalcon_update_property_empty_array(getThis(), SL("_defaultParams"));
}

/**
 * Sets the name of the default module
 *
 * @param string $moduleName
 */
PHP_METHOD(Phalcon_CLI_Router, setDefaultModule){

	zval *module_name;

	phalcon_fetch_params(0, 1, 0, &module_name);

	phalcon_update_property_this(getThis(), SL("_defaultModule"), module_name);

}

/**
 * Sets the name of the default namespace
 *
 * @param string $namespaceName
 */
PHP_METHOD(Phalcon_CLI_Router, setDefaultNamespace){

	zval *namespace_name;

	phalcon_fetch_params(0, 1, 0, &namespace_name);

	phalcon_update_property_this(getThis(), SL("_defaultNamespace"), namespace_name);

}

/**
 * Sets the default controller name
 *
 * @param string $taskName
 */
PHP_METHOD(Phalcon_CLI_Router, setDefaultTask){

	zval *task_name;

	phalcon_fetch_params(0, 1, 0, &task_name);

	phalcon_update_property_this(getThis(), SL("_defaultTask"), task_name);

}

/**
 * Sets the default action name
 *
 * @param string $actionName
 */
PHP_METHOD(Phalcon_CLI_Router, setDefaultAction){

	zval *action_name;

	phalcon_fetch_params(0, 1, 0, &action_name);

	phalcon_update_property_this(getThis(), SL("_defaultAction"), action_name);

}

/**
 * Handles routing information received from command-line arguments
 *
 * @param array $arguments
 */
PHP_METHOD(Phalcon_CLI_Router, handle){

	zval *arguments = NULL, module_name = {}, namespace_name = {}, task_name = {}, action_name = {};

	phalcon_fetch_params(0, 0, 1, &arguments);

	if (!arguments || Z_TYPE_P(arguments) != IS_ARRAY) { 
		PHALCON_THROW_EXCEPTION_STRW(phalcon_cli_router_exception_ce, "Arguments must be an Array");
		return;
	}

	PHALCON_SEPARATE_PARAM(arguments);

	/** 
	 * Check for a module
	 */
	if (phalcon_array_isset_fetch_str(&module_name, arguments, SL("module"))) {
		phalcon_array_unset_str(arguments, SL("module"), PH_COPY);
		phalcon_update_property_this(getThis(), SL("_module"), &module_name);
	}

	/**
	 * Check for a namespace
	 */
	if (phalcon_array_isset_fetch_str(&namespace_name, arguments, SL("namespace"))) {
		phalcon_array_unset_str(arguments, SL("namespace"), PH_COPY);
		phalcon_update_property_this(getThis(), SL("_namespace"), &namespace_name);
	}

	/** 
	 * Check for a task
	 */
	if (phalcon_array_isset_fetch_str(&task_name, arguments, SL("task"))) {
		phalcon_array_unset_str(arguments, SL("task"), PH_COPY);
		phalcon_update_property_this(getThis(), SL("_task"), &task_name);
	}

	/** 
	 * Check for an action
	 */
	if (phalcon_array_isset_fetch_str(&action_name, arguments, SL("action"))) {
		phalcon_array_unset_str(arguments, SL("action"), PH_COPY);
		phalcon_update_property_this(getThis(), SL("_action"), &action_name);
	}

	phalcon_update_property_this(getThis(), SL("_params"), arguments);
}

/**
 * Returns proccesed module name
 *
 * @return string
 */
PHP_METHOD(Phalcon_CLI_Router, getModuleName){


	RETURN_MEMBER(getThis(), "_module");
}

/**
 * Returns proccesed namespace name
 *
 * @return string
 */
PHP_METHOD(Phalcon_CLI_Router, getNamespaceName){


	RETURN_MEMBER(getThis(), "_namespace");
}

/**
 * Returns proccesed task name
 *
 * @return string
 */
PHP_METHOD(Phalcon_CLI_Router, getTaskName){


	RETURN_MEMBER(getThis(), "_task");
}

/**
 * Returns proccesed action name
 *
 * @return string
 */
PHP_METHOD(Phalcon_CLI_Router, getActionName){


	RETURN_MEMBER(getThis(), "_action");
}

/**
 * Returns proccesed extra params
 *
 * @return array
 */
PHP_METHOD(Phalcon_CLI_Router, getParams){


	RETURN_MEMBER(getThis(), "_params");
}

