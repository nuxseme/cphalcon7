
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
  |          ZhuZongXin <dreamsxin@qq.com>                                 |
  +------------------------------------------------------------------------+
*/

#include "mvc/collection/gridfs.h"
#include "mvc/collection.h"
#include "mvc/collection/exception.h"

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/fcall.h"
#include "kernel/exception.h"
#include "kernel/object.h"
#include "kernel/array.h"
#include "kernel/string.h"
#include "kernel/hash.h"
#include "kernel/operators.h"
#include "kernel/file.h"
#include "kernel/concat.h"
#include "kernel/variables.h"

#include "internal/arginfo.h"

/**
 * Phalcon\Mvc\Collection\GridFS
 *
 * This component implements a high level abstraction for NoSQL databases which
 * works with documents
 */
zend_class_entry *phalcon_mvc_collection_gridfs_ce;

PHP_METHOD(Phalcon_Mvc_Collection_GridFS, store);
PHP_METHOD(Phalcon_Mvc_Collection_GridFS, remove);
PHP_METHOD(Phalcon_Mvc_Collection_GridFS, save);
PHP_METHOD(Phalcon_Mvc_Collection_GridFS, saveBytes);
PHP_METHOD(Phalcon_Mvc_Collection_GridFS, create);
PHP_METHOD(Phalcon_Mvc_Collection_GridFS, createBytes);
PHP_METHOD(Phalcon_Mvc_Collection_GridFS, update);
PHP_METHOD(Phalcon_Mvc_Collection_GridFS, updateBytes);
PHP_METHOD(Phalcon_Mvc_Collection_GridFS, delete);
PHP_METHOD(Phalcon_Mvc_Collection_GridFS, drop);
PHP_METHOD(Phalcon_Mvc_Collection_GridFS, getFile);
PHP_METHOD(Phalcon_Mvc_Collection_GridFS, getBytes);
PHP_METHOD(Phalcon_Mvc_Collection_GridFS, write);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_collection_gridfs_storefile, 0, 0, 1)
	ZEND_ARG_INFO(0, filename)
	ZEND_ARG_INFO(0, metadata)
	ZEND_ARG_INFO(0, options)
	ZEND_ARG_INFO(0, isBytes)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_collection_gridfs_write, 0, 0, 1)
	ZEND_ARG_INFO(0, filename)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_mvc_collection_gridfs_method_entry[] = {
	PHP_ME(Phalcon_Mvc_Collection_GridFS, store, arginfo_phalcon_mvc_collection_gridfs_storefile, ZEND_ACC_PROTECTED)
	PHP_ME(Phalcon_Mvc_Collection_GridFS, remove, NULL, ZEND_ACC_PROTECTED)
	PHP_ME(Phalcon_Mvc_Collection_GridFS, save, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Collection_GridFS, saveBytes, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Collection_GridFS, create, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Collection_GridFS, createBytes, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Collection_GridFS, update, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Collection_GridFS, updateBytes, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Collection_GridFS, delete, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Collection_GridFS, drop, NULL, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Mvc_Collection_GridFS, getFile, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Collection_GridFS, getBytes, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Collection_GridFS, write, arginfo_phalcon_mvc_collection_gridfs_write, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\Mvc\Collection\GridFS initializer
 */
PHALCON_INIT_CLASS(Phalcon_Mvc_Collection_GridFS){

	PHALCON_REGISTER_CLASS_EX(Phalcon\\Mvc\\Collection, GridFS, mvc_collection_gridfs, phalcon_mvc_collection_ce, phalcon_mvc_collection_gridfs_method_entry, 0);

	zend_declare_property_null(phalcon_mvc_collection_gridfs_ce, SL("sha1"), ZEND_ACC_PUBLIC);
	zend_declare_property_null(phalcon_mvc_collection_gridfs_ce, SL("md5"), ZEND_ACC_PUBLIC);

	return SUCCESS;
}

PHP_METHOD(Phalcon_Mvc_Collection_GridFS, store){

	zval *file, *meta = NULL, *opts = NULL, *isBytes = NULL, metadata = {}, options = {}, mongo_id = {}, source = {}, files_source = {}, connection = {};
	zval mongo_collection = {}, *sha1, *md5, criteria = {}, operation = {}, field = {}, value = {}, new_object = {}, grid_fs = {}, status = {}, ok = {}, exist = {};

	phalcon_fetch_params(0, 1, 3, &file, &meta, &opts, &isBytes);

	if (!meta) {
		array_init_size(&metadata, 1);
	} else {
		PHALCON_CPY_WRT_CTOR(&metadata, meta);

		if (Z_TYPE(metadata) != IS_ARRAY) {
			array_init_size(&metadata, 1);
		}
	}

	if (!opts) {
		array_init_size(&options, 1);
	} else {
		PHALCON_CPY_WRT_CTOR(&options, opts);

		if (Z_TYPE(options) != IS_ARRAY) {
			array_init_size(&options, 1);
		}
	}

	if (!isBytes) {
		isBytes = &PHALCON_GLOBAL(z_false);
	}

	phalcon_array_update_str_long(&options, SL("w"), 0, 0);

	PHALCON_CALL_SELFW(&mongo_id, "getid");

	if (!zend_is_true(&mongo_id)) {
		RETURN_FALSE;
	}

	PHALCON_CALL_METHODW(&source, getThis(), "getsource");

	PHALCON_CONCAT_VS(&files_source, &source, ".files");

	PHALCON_CALL_METHODW(&connection, getThis(), "getconnection");
	PHALCON_CALL_METHODW(&mongo_collection, &connection, "selectcollection", &files_source);

	if (Z_TYPE(mongo_collection) != IS_OBJECT) {
		PHALCON_THROW_EXCEPTION_STRW(phalcon_mvc_collection_exception_ce, "Couldn't select mongo collection");
		return;
	}

	sha1 = phalcon_read_property(getThis(), SL("sha1"), PH_NOISY);
	md5 = phalcon_read_property(getThis(), SL("md5"), PH_NOISY);

	array_init_size(&criteria, 3);

	phalcon_array_update_str(&criteria, SL("md5"), md5, PH_COPY);
	phalcon_array_update_str(&criteria, SL("sha1"), sha1, PH_COPY);

	ZVAL_STRING(&operation, "$gte");
	ZVAL_STRING(&field, "use");
	ZVAL_LONG(&value, 1)

	phalcon_array_update_multi_2(&criteria, &field, &operation, &value, PH_COPY);

	ZVAL_STRING(&operation, "$inc");

	array_init_size(&new_object, 1);

	phalcon_array_update_multi_2(&new_object, &operation, &field, &value, PH_COPY);

	PHALCON_CALL_METHODW(&status, &mongo_collection, "update", &criteria, &new_object);

	if (phalcon_array_isset_fetch_str(&ok, &status, SL("ok"))) {
		if (zend_is_true(&ok)) {
			if (phalcon_array_isset_fetch_str(&exist, &status, SL("updatedExisting"))) {
				if (zend_is_true(&exist)) {
					RETURN_TRUE;
				}
			}
		} else {
			RETURN_FALSE;
		}
	}

	PHALCON_CALL_METHODW(&grid_fs, &connection, "getgridfs", &source);
	if (Z_TYPE(grid_fs) != IS_OBJECT) {
		PHALCON_THROW_EXCEPTION_STRW(phalcon_mvc_collection_exception_ce, "Couldn't select mongo GridFS");
		return;
	}

	phalcon_array_update_str(&metadata, SL("sha1"), sha1, PH_COPY);
	phalcon_array_update_str(&metadata, SL("use"), &value, PH_COPY);

	if (zend_is_true(isBytes)) {
		PHALCON_CALL_METHODW(&status, &grid_fs, "storebytes", file, &metadata, &options);
	} else {
		PHALCON_CALL_METHODW(&status, &grid_fs, "storefile", file, &metadata, &options);
	}

	if (zend_is_true(&status)) {
		RETURN_TRUE;
	}

	RETURN_FALSE;
}

PHP_METHOD(Phalcon_Mvc_Collection_GridFS, remove){

	zval *sha1 = NULL, *md5 = NULL, source = {}, files_source = {}, connection = {}, mongo_collection = {}, criteria = {}, operation = {}, field = {}, value = {}, new_object = {};
	zval status = {}, ok = {}, exist = {}, options = {}, grid_fs = {};

	phalcon_fetch_params(0, 0, 2, &sha1, &md5);

	if (!sha1) {
		sha1 = phalcon_read_property(getThis(), SL("sha1"), PH_NOISY);
	}

	if (!md5) {
		md5 = phalcon_read_property(getThis(), SL("md5"), PH_NOISY);
	}

	PHALCON_CALL_METHODW(&source, getThis(), "getsource");

	PHALCON_CONCAT_VS(&files_source, &source, ".files");

	PHALCON_CALL_METHODW(&connection, getThis(), "getconnection");
	PHALCON_CALL_METHODW(&mongo_collection, &connection, "selectcollection", &files_source);

	if (Z_TYPE(mongo_collection) != IS_OBJECT) {
		PHALCON_THROW_EXCEPTION_STRW(phalcon_mvc_collection_exception_ce, "Couldn't select mongo collection");
		return;
	}

	array_init_size(&criteria, 2);

	phalcon_array_update_str(&criteria, SL("md5"), md5, PH_COPY);
	phalcon_array_update_str(&criteria, SL("sha1"), sha1, PH_COPY);

	ZVAL_STRING(&operation, "$inc");
	ZVAL_STRING(&field, "use");
	ZVAL_LONG(&value, -1)

	array_init_size(&new_object, 1);

	phalcon_array_update_multi_2(&new_object, &operation, &field, &value, PH_COPY);

	PHALCON_CALL_METHODW(&status, &mongo_collection, "update", &criteria, &new_object);

	if (phalcon_array_isset_fetch_str(&ok, &status, SL("ok"))) {
		if (zend_is_true(&ok)) {
			if (phalcon_array_isset_fetch_str(&exist, &status, SL("updatedExisting"))) {
				if (!zend_is_true(&exist)) {
					RETURN_FALSE;
				}
			}
		} else {
			RETURN_FALSE;
		}
	}

	PHALCON_CALL_METHODW(&grid_fs, &connection, "getgridfs", &source);

	if (Z_TYPE(grid_fs) != IS_OBJECT) {
		PHALCON_THROW_EXCEPTION_STRW(phalcon_mvc_collection_exception_ce, "Couldn't select mongo GridFS");
		return;
	}

	array_init_size(&criteria, 3);

	phalcon_array_update_str(&criteria, SL("sha1"), sha1, PH_COPY);
	phalcon_array_update_str(&criteria, SL("md5"), md5, PH_COPY);

	ZVAL_STRING(&operation, "$lte");

	ZVAL_LONG(&value, 0)

	phalcon_array_update_multi_2(&criteria, &field, &operation, &value, PH_COPY);

	array_init_size(&options, 1);

	phalcon_array_update_str_long(&options, SL("w"), 0, PH_COPY);

	PHALCON_RETURN_CALL_METHODW(&grid_fs, "remove", &criteria, &options);
}

PHP_METHOD(Phalcon_Mvc_Collection_GridFS, save){

	zval *filename, *arr = NULL, *white_list = NULL, *mode = NULL, *old_sha1, *old_md5, sha1 = {}, md5 = {}, status = {};

	phalcon_fetch_params(0, 1, 3, &filename, &arr, &white_list, &mode);

	if (!arr) {
		arr = &PHALCON_GLOBAL(z_null);
	}

	if (!white_list) {
		white_list = &PHALCON_GLOBAL(z_null);
	}

	if (!mode) {
		mode = &PHALCON_GLOBAL(z_null);
	}

	if (zend_is_true(filename)) {
		old_sha1 = phalcon_read_property(getThis(), SL("sha1"), PH_NOISY);
		old_md5 = phalcon_read_property(getThis(), SL("md5"), PH_NOISY);

		PHALCON_CALL_FUNCTIONW(&sha1, "sha1_file", filename);
		PHALCON_CALL_FUNCTIONW(&md5, "md5_file", filename);

		phalcon_update_property_this(getThis(), SL("sha1"), &sha1);
		phalcon_update_property_this(getThis(), SL("md5"), &md5);
	}

	PHALCON_CALL_PARENTW(&status, phalcon_mvc_collection_gridfs_ce, getThis(), "save", arr, white_list, mode);

	if (PHALCON_IS_FALSE(&status)) {
		RETURN_FALSE;
	}

	if (zend_is_true(filename)) {
		PHALCON_CALL_SELFW(&status, "store", filename);
		PHALCON_CALL_SELFW(NULL, "remove", old_sha1, old_md5);
	}

	RETURN_CTORW(&status);
}

PHP_METHOD(Phalcon_Mvc_Collection_GridFS, saveBytes){

	zval *bytes, *arr = NULL, *white_list = NULL, *mode = NULL, *old_sha1, *old_md5, sha1 = {}, md5 = {}, status = {};

	phalcon_fetch_params(0, 1, 3, &bytes, &arr, &white_list, &mode);

	if (!arr) {
		arr = &PHALCON_GLOBAL(z_null);
	}

	if (!white_list) {
		white_list = &PHALCON_GLOBAL(z_null);
	}

	if (!mode) {
		mode = &PHALCON_GLOBAL(z_null);
	}

	if (zend_is_true(bytes)) {
		old_sha1 = phalcon_read_property(getThis(), SL("sha1"), PH_NOISY);
		old_md5 = phalcon_read_property(getThis(), SL("md5"), PH_NOISY);

		PHALCON_CALL_FUNCTIONW(&sha1, "sha1", bytes);
		PHALCON_CALL_FUNCTIONW(&md5, "md5", bytes);

		phalcon_update_property_this(getThis(), SL("sha1"), &sha1);
		phalcon_update_property_this(getThis(), SL("md5"), &md5);
	}

	PHALCON_CALL_PARENTW(&status, phalcon_mvc_collection_gridfs_ce, getThis(), "save", arr, white_list, mode);

	if (PHALCON_IS_FALSE(&status)) {
		RETURN_FALSE;
	}

	if (zend_is_true(bytes)) {
		PHALCON_CALL_SELFW(&status, "store", bytes, &PHALCON_GLOBAL(z_null), &PHALCON_GLOBAL(z_null), &PHALCON_GLOBAL(z_true));
		if (zend_is_true(old_sha1)) {
			PHALCON_CALL_SELFW(NULL, "remove", old_sha1, old_md5);
		}
	}

	RETURN_CTORW(&status);
}

PHP_METHOD(Phalcon_Mvc_Collection_GridFS, create){

	zval *filename = NULL, *data = NULL, *white_list = NULL;

	phalcon_fetch_params(0, 0, 3, &filename, &data, &white_list);

	if (!filename) {
		filename = &PHALCON_GLOBAL(z_null);
	}

	if (!data) {
		data = &PHALCON_GLOBAL(z_null);
	}

	if (!white_list) {
		white_list = &PHALCON_GLOBAL(z_null);
	}

	PHALCON_RETURN_CALL_METHODW(getThis(), "save", filename, data, white_list, &PHALCON_GLOBAL(z_true));
}

PHP_METHOD(Phalcon_Mvc_Collection_GridFS, createBytes){

	zval *bytes = NULL, *data = NULL, *white_list = NULL;

	phalcon_fetch_params(0, 0, 3, &bytes, &data, &white_list);

	if (!bytes) {
		bytes = &PHALCON_GLOBAL(z_null);
	}

	if (!data) {
		data = &PHALCON_GLOBAL(z_null);
	}

	if (!white_list) {
		white_list = &PHALCON_GLOBAL(z_null);
	}

	PHALCON_RETURN_CALL_METHODW(getThis(), "savebytes", bytes, data, white_list, &PHALCON_GLOBAL(z_true));
}

PHP_METHOD(Phalcon_Mvc_Collection_GridFS, update){

	zval *filename = NULL, *data = NULL, *white_list = NULL, *isBytes = NULL;

	phalcon_fetch_params(0, 0, 4, &filename, &data, &white_list, &isBytes);

	if (!filename) {
		filename = &PHALCON_GLOBAL(z_null);
	}

	if (!data) {
		data = &PHALCON_GLOBAL(z_null);
	}

	if (!white_list) {
		white_list = &PHALCON_GLOBAL(z_null);
	}

	PHALCON_RETURN_CALL_METHODW(getThis(), "save", filename, data, white_list, &PHALCON_GLOBAL(z_false));
}

PHP_METHOD(Phalcon_Mvc_Collection_GridFS, updateBytes){

	zval *bytes = NULL, *data = NULL, *white_list = NULL;

	phalcon_fetch_params(0, 0, 3, &bytes, &data, &white_list);

	if (!bytes) {
		bytes = &PHALCON_GLOBAL(z_null);
	}

	if (!data) {
		data = &PHALCON_GLOBAL(z_null);
	}

	if (!white_list) {
		white_list = &PHALCON_GLOBAL(z_null);
	}

	PHALCON_RETURN_CALL_METHODW(getThis(), "saveBytes", bytes, data, white_list, &PHALCON_GLOBAL(z_false));
}

PHP_METHOD(Phalcon_Mvc_Collection_GridFS, delete){

	zval status = {};

	PHALCON_CALL_PARENTW(&status, phalcon_mvc_collection_gridfs_ce, getThis(), "delete");
	if (PHALCON_IS_FALSE(&status)) {
		RETURN_FALSE;
	}

	PHALCON_RETURN_CALL_METHODW(getThis(), "remove");
}

PHP_METHOD(Phalcon_Mvc_Collection_GridFS, drop){

	zval class_name = {}, collection = {}, ok = {}, source = {}, connection = {}, mongo_collection = {}, status = {}, grid_fs = {};
	zend_class_entry *ce0;

	phalcon_get_called_class(&class_name);
	ce0 = phalcon_fetch_class(&class_name, ZEND_FETCH_CLASS_DEFAULT);

	object_init_ex(&collection, ce0);
	if (phalcon_has_constructor(&collection)) {
		PHALCON_CALL_METHODW(NULL, &collection, "__construct");
	}

	ZVAL_FALSE(&ok);

	PHALCON_CALL_METHODW(&source, &collection, "getsource");
	PHALCON_CALL_METHODW(&connection, &collection, "getconnection");
	PHALCON_CALL_METHODW(&mongo_collection, &connection, "selectcollection", &source);

	if (Z_TYPE(mongo_collection) == IS_OBJECT) {
		PHALCON_CALL_METHODW(&status, &mongo_collection, "drop");

		if (phalcon_array_isset_str(&status, SL("ok"))) {
			phalcon_array_fetch_str(&ok, &status, SL("ok"), PH_NOISY);
		}
	}

	PHALCON_CALL_METHODW(&grid_fs, &connection, "getgridfs", &source);
	if (Z_TYPE(grid_fs) == IS_OBJECT) {
		PHALCON_CALL_METHODW(&status, &grid_fs, "drop");
		if (phalcon_array_isset_str(&status, SL("ok"))) {
			phalcon_array_fetch_str(&ok, &status, SL("ok"), PH_NOISY);
		}
	}

	if (zend_is_true(&ok)) {
		RETURN_TRUE;
	}

	RETURN_FALSE;
}

PHP_METHOD(Phalcon_Mvc_Collection_GridFS, getFile){

	zval mongo_id = {}, source = {}, connection = {}, grid_fs = {}, *sha1, *md5, criteria = {};

	PHALCON_CALL_SELFW(&mongo_id, "getid");

	if (!zend_is_true(&mongo_id)) {
		RETURN_FALSE;
	}

	PHALCON_CALL_METHODW(&source, getThis(), "getsource");
	PHALCON_CALL_METHODW(&connection, getThis(), "getconnection");
	PHALCON_CALL_METHODW(&grid_fs, &connection, "getgridfs", &source);

	if (Z_TYPE(grid_fs) != IS_OBJECT) {
		PHALCON_THROW_EXCEPTION_STRW(phalcon_mvc_collection_exception_ce, "Couldn't select mongo GridFS");
		return;
	}

	sha1 = phalcon_read_property(getThis(), SL("sha1"), PH_NOISY);
	md5 = phalcon_read_property(getThis(), SL("md5"), PH_NOISY);

	array_init_size(&criteria, 2);

	phalcon_array_update_str(&criteria, SL("md5"), md5, PH_COPY);
	phalcon_array_update_str(&criteria, SL("sha1"), sha1, PH_COPY);

	PHALCON_RETURN_CALL_METHODW(&grid_fs, "findone", &criteria);
}

PHP_METHOD(Phalcon_Mvc_Collection_GridFS, getBytes){

	zval file = {};

	PHALCON_CALL_METHODW(&file, getThis(), "getfile");

	if (Z_TYPE(file) == IS_OBJECT) {
		PHALCON_RETURN_CALL_METHODW(&file, "getbytes");
	} else {
		RETURN_FALSE;
	}
}

PHP_METHOD(Phalcon_Mvc_Collection_GridFS, write){

	zval *filename, file = {};

	phalcon_fetch_params(0, 1, 0, &filename);

	PHALCON_CALL_METHODW(&file, getThis(), "getfile");

	if (Z_TYPE(file) == IS_OBJECT) {
		PHALCON_RETURN_CALL_METHODW(&file, "write", filename);
	} else {
		RETURN_FALSE;
	}
}
