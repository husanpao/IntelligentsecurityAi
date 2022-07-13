/*----------------------------------------------------------------------------------------------
*
* This file is ArcSoft's property. It contains ArcSoft's trade secret, proprietary and
* confidential information.
*
* The information and code contained in this file is only for authorized ArcSoft employees
* to design, create, modify, or review.
*
* DO NOT DISTRIBUTE, DO NOT DUPLICATE OR TRANSMIT IN ANY FORM WITHOUT PROPER AUTHORIZATION.
*
* If you are not an intended recipient of this file, you must not copy, distribute, modify,
* or take any action in reliance on it.
*
* If you have received this file in error, please immediately notify ArcSoft and
* permanently delete the original and any copy of any file and any printout thereof.
*
*-------------------------------------------------------------------------------------------------*/


#ifndef __MERROR_H__
#define __MERROR_H__


#define MERR_NONE						(0)
#define MOK								(0)

#define MERR_BASIC_BASE					0X0001							//ͨ�ô�������
#define MERR_UNKNOWN					MERR_BASIC_BASE					//����ԭ����
#define MERR_INVALID_PARAM				(MERR_BASIC_BASE+1)				//��Ч�Ĳ���
#define MERR_UNSUPPORTED				(MERR_BASIC_BASE+2)				//���治֧��
#define MERR_NO_MEMORY					(MERR_BASIC_BASE+3)				//�ڴ治��
#define MERR_BAD_STATE					(MERR_BASIC_BASE+4)				//״̬����
#define MERR_USER_CANCEL				(MERR_BASIC_BASE+5)				//�û�ȡ����ز���
#define MERR_EXPIRED					(MERR_BASIC_BASE+6)				//����ʱ�����
#define MERR_USER_PAUSE					(MERR_BASIC_BASE+7)				//�û���ͣ����
#define MERR_BUFFER_OVERFLOW		    (MERR_BASIC_BASE+8)				//��������
#define MERR_BUFFER_UNDERFLOW		    (MERR_BASIC_BASE+9)				//��������
#define MERR_NO_DISKSPACE				(MERR_BASIC_BASE+10)			//�����ռ䲻��
#define	MERR_COMPONENT_NOT_EXIST		(MERR_BASIC_BASE+11)			//���������
#define	MERR_GLOBAL_DATA_NOT_EXIST		(MERR_BASIC_BASE+12)			//ȫ�����ݲ�����
#define MERRP_IMGCODEC					(MERR_BASIC_BASE+13)			//ͼ�����
#define MERR_FILE_GENERAL				(MERR_BASIC_BASE+14)            //�ļ�����

#define MERR_FSDK_BASE							0X7000					//Free SDKͨ�ô�������
#define MERR_FSDK_INVALID_APP_ID				(MERR_FSDK_BASE+1)		//��Ч��App Id
#define MERR_FSDK_INVALID_SDK_ID				(MERR_FSDK_BASE+2)		//��Ч��SDK key
#define MERR_FSDK_INVALID_ID_PAIR				(MERR_FSDK_BASE+3)		//AppId��SDKKey��ƥ��
#define MERR_FSDK_MISMATCH_ID_AND_SDK			(MERR_FSDK_BASE+4)		//SDKKey ��ʹ�õ�SDK ��ƥ��
#define MERR_FSDK_SYSTEM_VERSION_UNSUPPORTED	(MERR_FSDK_BASE+5)		//ϵͳ�汾������ǰSDK��֧��
#define MERR_FSDK_LICENCE_EXPIRED				(MERR_FSDK_BASE+6)		//SDK��Ч�ڹ��ڣ���Ҫ�������ظ���

#define MERR_FSDK_APS_ERROR_BASE				0x11000							//PhotoStyling ��������
#define MERR_FSDK_APS_ENGINE_HANDLE				(MERR_FSDK_APS_ERROR_BASE+1)	//�������Ƿ�
#define MERR_FSDK_APS_MEMMGR_HANDLE				(MERR_FSDK_APS_ERROR_BASE+2)	//�ڴ����Ƿ�
#define MERR_FSDK_APS_DEVICEID_INVALID			(MERR_FSDK_APS_ERROR_BASE+3)	//Device ID �Ƿ�
#define MERR_FSDK_APS_DEVICEID_UNSUPPORTED		(MERR_FSDK_APS_ERROR_BASE+4)	//Device ID ��֧��
#define MERR_FSDK_APS_MODEL_HANDLE				(MERR_FSDK_APS_ERROR_BASE+5)	//ģ������ָ��Ƿ�
#define MERR_FSDK_APS_MODEL_SIZE				(MERR_FSDK_APS_ERROR_BASE+6)	//ģ�����ݳ��ȷǷ�
#define MERR_FSDK_APS_IMAGE_HANDLE              (MERR_FSDK_APS_ERROR_BASE+7)	//ͼ��ṹ��ָ��Ƿ�
#define MERR_FSDK_APS_IMAGE_FORMAT_UNSUPPORTED  (MERR_FSDK_APS_ERROR_BASE+8)	//ͼ���ʽ��֧��
#define MERR_FSDK_APS_IMAGE_PARAM               (MERR_FSDK_APS_ERROR_BASE+9)	//ͼ������Ƿ�
#define MERR_FSDK_APS_IMAGE_SIZE				(MERR_FSDK_APS_ERROR_BASE+10)	//ͼ��ߴ��С����֧�ַ�Χ
#define MERR_FSDK_APS_DEVICE_AVX2_UNSUPPORTED	(MERR_FSDK_APS_ERROR_BASE+11)	//��������֧��AVX2ָ��

#define MERR_FSDK_FR_ERROR_BASE					0x12000							//Face Recognition��������
#define MERR_FSDK_FR_INVALID_MEMORY_INFO		(MERR_FSDK_FR_ERROR_BASE+1)		//��Ч�������ڴ�
#define MERR_FSDK_FR_INVALID_IMAGE_INFO			(MERR_FSDK_FR_ERROR_BASE+2)		//��Ч������ͼ�����
#define MERR_FSDK_FR_INVALID_FACE_INFO			(MERR_FSDK_FR_ERROR_BASE+3)		//��Ч��������Ϣ
#define MERR_FSDK_FR_NO_GPU_AVAILABLE			(MERR_FSDK_FR_ERROR_BASE+4)		//��ǰ�豸��GPU����
#define MERR_FSDK_FR_MISMATCHED_FEATURE_LEVEL	(MERR_FSDK_FR_ERROR_BASE+5)		//���Ƚϵ��������������İ汾��һ��

#define MERR_FSDK_FACEFEATURE_ERROR_BASE			0x14000									//������������������
#define MERR_FSDK_FACEFEATURE_UNKNOWN				(MERR_FSDK_FACEFEATURE_ERROR_BASE+1)	//��������������δ֪
#define MERR_FSDK_FACEFEATURE_MEMORY				(MERR_FSDK_FACEFEATURE_ERROR_BASE+2)	//������������ڴ����
#define MERR_FSDK_FACEFEATURE_INVALID_FORMAT		(MERR_FSDK_FACEFEATURE_ERROR_BASE+3)	//������������ʽ����
#define MERR_FSDK_FACEFEATURE_INVALID_PARAM			(MERR_FSDK_FACEFEATURE_ERROR_BASE+4)	//������������������
#define MERR_FSDK_FACEFEATURE_LOW_CONFIDENCE_LEVEL	(MERR_FSDK_FACEFEATURE_ERROR_BASE+5)	//����������������Ŷȵ�
#define MERR_FSDK_FACEFEATURE_EXPIRED	            (MERR_FSDK_FACEFEATURE_ERROR_BASE+6)	//���������������������
#define MERR_FSDK_FACEFEATURE_MISSFACE	            (MERR_FSDK_FACEFEATURE_ERROR_BASE+7)	//�����������������ʧ
#define MERR_FSDK_FACEFEATURE_NO_FACE               (MERR_FSDK_FACEFEATURE_ERROR_BASE+8)	//�����������û������
#define MERR_FSDK_FACEFEATURE_FACEDATD              (MERR_FSDK_FACEFEATURE_ERROR_BASE+9)	//�����������������Ϣ����
#define MERR_FSDK_FACEFEATURE_STATUES_ERROR			(MERR_FSDK_FACEFEATURE_ERROR_BASE+10)   //���������������״̬����

#define MERR_ASF_EX_BASE								0x15000							//ASF��������
#define MERR_ASF_EX_FEATURE_UNSUPPORTED_ON_INIT			(MERR_ASF_EX_BASE+1)			//Engine��֧�ֵļ������
#define MERR_ASF_EX_FEATURE_UNINITED					(MERR_ASF_EX_BASE+2)			//��Ҫ��������δ��ʼ��
#define MERR_ASF_EX_FEATURE_UNPROCESSED					(MERR_ASF_EX_BASE+3)			//����ȡ������δ��process�д����
#define MERR_ASF_EX_FEATURE_UNSUPPORTED_ON_PROCESS		(MERR_ASF_EX_BASE+4)			//PROCESS��֧�ֵļ��������ϣ�����FR�����Լ������Ĵ�����
#define MERR_ASF_EX_INVALID_IMAGE_INFO					(MERR_ASF_EX_BASE+5)			//��Ч������ͼ��
#define MERR_ASF_EX_INVALID_FACE_INFO					(MERR_ASF_EX_BASE+6)			//��Ч��������Ϣ


#define MERR_ASF_BASE									0x16000							//�����ȶԻ�����������
#define MERR_ASF_ACTIVATION_FAIL						(MERR_ASF_BASE+1)				//SDK����ʧ��,��򿪶�дȨ��
#define MERR_ASF_ALREADY_ACTIVATED						(MERR_ASF_BASE+2)				//SDK�Ѽ���
#define MERR_ASF_NOT_ACTIVATED							(MERR_ASF_BASE+3)				//SDKδ����
#define MERR_ASF_SCALE_NOT_SUPPORT						(MERR_ASF_BASE+4)				//detectFaceScaleVal ��֧��
#define MERR_ASF_ACTIVEFILE_SDKTYPE_MISMATCH			(MERR_ASF_BASE+5)				//�����ļ���SDK���Ͳ�ƥ�䣬��ȷ��ʹ�õ�sdk
#define MERR_ASF_DEVICE_MISMATCH						(MERR_ASF_BASE+6)				//�豸��ƥ��
#define MERR_ASF_UNIQUE_IDENTIFIER_ILLEGAL				(MERR_ASF_BASE+7)				//Ψһ��ʶ���Ϸ�
#define MERR_ASF_PARAM_NULL								(MERR_ASF_BASE+8)				//����Ϊ��
#define MERR_ASF_LIVENESS_EXPIRED						(MERR_ASF_BASE+9)				//�����ѹ���
#define MERR_ASF_VERSION_NOT_SUPPORT					(MERR_ASF_BASE+10)				//�汾��֧��
#define MERR_ASF_SIGN_ERROR								(MERR_ASF_BASE+11)				//ǩ������
#define MERR_ASF_DATABASE_ERROR							(MERR_ASF_BASE+12)				//������Ϣ�����쳣
#define MERR_ASF_UNIQUE_CHECKOUT_FAIL					(MERR_ASF_BASE+13)				//Ψһ��ʶ��У��ʧ��
#define MERR_ASF_COLOR_SPACE_NOT_SUPPORT				(MERR_ASF_BASE+14)				//��ɫ�ռ䲻֧��
#define	MERR_ASF_IMAGE_WIDTH_HEIGHT_NOT_SUPPORT			(MERR_ASF_BASE+15)				//ͼƬ��߲�֧�֣���������ֽڶ���

#define MERR_ASF_BASE_EXTEND							0x16010							//�����ȶԻ�����������
#define MERR_ASF_READ_PHONE_STATE_DENIED				(MERR_ASF_BASE_EXTEND)			//android.permission.READ_PHONE_STATEȨ�ޱ��ܾ�
#define	MERR_ASF_ACTIVATION_DATA_DESTROYED				(MERR_ASF_BASE_EXTEND+1)		//�������ݱ��ƻ�,��ɾ�������ļ������½��м���
#define	MERR_ASF_SERVER_UNKNOWN_ERROR					(MERR_ASF_BASE_EXTEND+2)		//�����δ֪����
#define MERR_ASF_INTERNET_DENIED				        (MERR_ASF_BASE_EXTEND+3)		//INTERNETȨ�ޱ��ܾ�
#define MERR_ASF_ACTIVEFILE_SDK_MISMATCH				(MERR_ASF_BASE_EXTEND+4)		//�����ļ���SDK�汾��ƥ��,�����¼���
#define MERR_ASF_DEVICEINFO_LESS						(MERR_ASF_BASE_EXTEND+5)		//�豸��Ϣ̫�٣������������豸ָ��
#define MERR_ASF_LOCAL_TIME_NOT_CALIBRATED				(MERR_ASF_BASE_EXTEND+6)		//�ͻ���ʱ���������ʱ�䣨������ʱ�䣩ǰ�������30��������
#define MERR_ASF_APPID_DATA_DECRYPT						(MERR_ASF_BASE_EXTEND+7)		//����У���쳣
#define MERR_ASF_APPID_APPKEY_SDK_MISMATCH				(MERR_ASF_BASE_EXTEND+8)		//�����AppId��AppKey��ʹ�õ�SDK�汾��һ��
#define MERR_ASF_NO_REQUEST								(MERR_ASF_BASE_EXTEND+9)		//��ʱ���������ᱻ��ֹ����,30����֮����
#define MERR_ASF_ACTIVE_FILE_NO_EXIST					(MERR_ASF_BASE_EXTEND+10)		//�����ļ�������
#define MERR_ASF_DETECT_MODEL_UNSUPPORTED				(MERR_ASF_BASE_EXTEND+11)		//���ģ�Ͳ�֧�֣���鿴��Ӧ�ӿ�˵����ʹ�õ�ǰ֧�ֵļ��ģ��
#define MERR_ASF_CURRENT_DEVICE_TIME_INCORRECT			(MERR_ASF_BASE_EXTEND+12)		//��ǰ�豸ʱ�䲻��ȷ��������豸ʱ��
#define MERR_ASF_ACTIVATION_QUANTITY_OUT_OF_LIMIT		(MERR_ASF_BASE_EXTEND+13)		//��ȼ��������������ƣ���������

#define MERR_ASF_NETWORK_BASE							0x17000							//�����������
#define MERR_ASF_NETWORK_COULDNT_RESOLVE_HOST			(MERR_ASF_NETWORK_BASE+1)		//�޷�����������ַ
#define MERR_ASF_NETWORK_COULDNT_CONNECT_SERVER			(MERR_ASF_NETWORK_BASE+2)		//�޷����ӷ�����
#define MERR_ASF_NETWORK_CONNECT_TIMEOUT				(MERR_ASF_NETWORK_BASE+3)		//�������ӳ�ʱ
#define MERR_ASF_NETWORK_UNKNOWN_ERROR					(MERR_ASF_NETWORK_BASE+4)		//����δ֪����

#define MERR_ASF_ACTIVEKEY_BASE							0x18000							//�������������
#define MERR_ASF_ACTIVEKEY_COULDNT_CONNECT_SERVER		(MERR_ASF_ACTIVEKEY_BASE+1)		//�޷����Ӽ��������
#define MERR_ASF_ACTIVEKEY_SERVER_SYSTEM_ERROR			(MERR_ASF_ACTIVEKEY_BASE+2)		//������ϵͳ����
#define MERR_ASF_ACTIVEKEY_POST_PARM_ERROR				(MERR_ASF_ACTIVEKEY_BASE+3)		//�����������
#define MERR_ASF_ACTIVEKEY_PARM_MISMATCH				(MERR_ASF_ACTIVEKEY_BASE+4)		//ACTIVEKEY��APPID��SDKKEY��ƥ��
#define MERR_ASF_ACTIVEKEY_ACTIVEKEY_ACTIVATED			(MERR_ASF_ACTIVEKEY_BASE+5)		//ACTIVEKEY�Ѿ���ʹ��
#define MERR_ASF_ACTIVEKEY_ACTIVEKEY_FORMAT_ERROR		(MERR_ASF_ACTIVEKEY_BASE+6)		//ACTIVEKEY��Ϣ�쳣
#define MERR_ASF_ACTIVEKEY_APPID_PARM_MISMATCH			(MERR_ASF_ACTIVEKEY_BASE+7)		//ACTIVEKEY��APPID��ƥ��
#define MERR_ASF_ACTIVEKEY_SDK_FILE_MISMATCH			(MERR_ASF_ACTIVEKEY_BASE+8)		//SDK�뼤���ļ��汾��ƥ��
#define MERR_ASF_ACTIVEKEY_EXPIRED						(MERR_ASF_ACTIVEKEY_BASE+9)		//ACTIVEKEY�ѹ���
#define MERR_ASF_ACTIVEKEY_DEVICE_OUT_OF_LIMIT			(MERR_ASF_ACTIVEKEY_BASE+10)	//������Ȩ�������豸����������


#define MERR_ASF_OFFLINE_BASE							0x19000							//���߼������������
#define MERR_ASF_LICENSE_FILE_NOT_EXIST					(MERR_ASF_OFFLINE_BASE+1)		//������Ȩ�ļ������ڻ��޶�дȨ��
#define MERR_ASF_LICENSE_FILE_DATA_DESTROYED			(MERR_ASF_OFFLINE_BASE+2)		//������Ȩ�ļ�����
#define MERR_ASF_LICENSE_FILE_SDK_MISMATCH				(MERR_ASF_OFFLINE_BASE+3)		//������Ȩ�ļ���SDK�汾��ƥ��
#define MERR_ASF_LICENSE_FILEINFO_SDKINFO_MISMATCH		(MERR_ASF_OFFLINE_BASE+4)		//������Ȩ�ļ���SDK��Ϣ��ƥ��
#define MERR_ASF_LICENSE_FILE_FINGERPRINT_MISMATCH		(MERR_ASF_OFFLINE_BASE+5)		//������Ȩ�ļ����豸ָ�Ʋ�ƥ��
#define MERR_ASF_LICENSE_FILE_EXPIRED					(MERR_ASF_OFFLINE_BASE+6)		//������Ȩ�ļ��ѹ���
#define MERR_ASF_LOCAL_EXIST_USEFUL_ACTIVE_FILE			(MERR_ASF_OFFLINE_BASE+7)		//������Ȩ�ļ������ã�����ԭ�м����ļ��ɼ���ʹ��
#define MERR_ASF_LICENSE_FILE_VERSION_TOO_LOW			(MERR_ASF_OFFLINE_BASE+8)		//������Ȩ�ļ��汾���ͣ���ʹ���°汾�����������½������߼���

#define MERR_ASF_SEARCH_BASE							0x25000							//�����ӿڴ���������
#define MERR_ASF_SEARCH_EMPTY                           (MERR_ASF_SEARCH_BASE+1)        //�����б�Ϊ��
#define MERR_ASF_SEARCH_NO_EXIST                        (MERR_ASF_SEARCH_BASE+2)        //����������
#define MERR_ASF_SEARCH_FEATURE_SIZE_MISMATCH           (MERR_ASF_SEARCH_BASE+3)        //����ֵ���Ȳ�ƥ��
#define MERR_ASF_SEARCH_LOW_CONFIDENCE                  (MERR_ASF_SEARCH_BASE+4)        //���ƶ��쳣

#define MERR_ASF_RESERVED_BASE                          0x30000                         //Ԥ���ֶδ���������
#define MERR_ASF_UNRESTRICTED_TIME_LIMIT_EXCEEDED       (MERR_ASF_RESERVED_BASE+1)      //������ʱ����������(����ʱ�Σ���һ������ 9��00-14��00)


#endif

