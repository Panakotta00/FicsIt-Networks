pipeline {
	agent {
		label 'Windows2019'
	}

	options {
		disableConcurrentBuilds()
		skipDefaultCheckout(true)
	}

	environment {
        MOD_NAME = 'FicsItNetworks'
    }


	stages {
		stage('SML') {
			steps {
				checkout scm: [
	                $class: 'GitSCM',
	                branches: [[
	                    name: "07f4ae52099642e2fba016ae9642df41bf164749"
	                ]],
	                extensions: [[
	                    $class: 'RelativeTargetDirectory',
	                    relativeTargetDir: 'SatisfactoryModLoader'
	                ]],
	                userRemoteConfigs: [[
	                    url: 'https://github.com/satisfactorymodding/SatisfactoryModLoader.git'
	                ]]
	            ]
	        }
		}

		stage('Checkout') {
			steps {
				dir("SatisfactoryModLoader/Plugins") {
					checkout scm: [
						$class: 'GitSCM',
						branches: scm.branches,
						extensions: [[
							$class: 'RelativeTargetDirectory',
							relativeTargetDir: '${MOD_NAME}'
						]],
						submoduleCfg: scm.submoduleCfg,
						doGenerateSubmoduleConfigurations: scm.doGenerateSubmoduleConfigurations,
						userRemoteConfigs: scm.userRemoteConfigs
					]
				}
			}
		}

		stage('Apply Patches') {
			steps {
				dir("SatisfactoryModLoader") {
					bat label: 'Apply SML Patch', script: 'git apply Plugins\\%MOD_NAME%\\SML_Patch.patch -v'
					bat label: 'Apply Asset Patch', script: 'git apply %ASSETS% -v'
					bat label: 'Add WWise', script: '7z x %WWISE_PLUGIN% -oPlugins\\'
				}
			}
		}

		stage('Setup UE4') {
			steps {
				dir('ue4') {
					withCredentials([string(credentialsId: 'GitHub-API', variable: 'GITHUB_TOKEN')]) {
						retry(3) {
							bat label: 'Download UE', script: 'github-release download --user SatisfactoryModdingUE --repo UnrealEngine -l -n "UnrealEngine-CSS-Editor-Win64.zip" > UnrealEngine-CSS-Editor-Win64.zip'
						}
						bat label: 'Extract UE', script: '7z x UnrealEngine-CSS-Editor-Win64.zip'
					}
					bat label: 'Register UE', script: 'SetupScripts\\Register.bat'
				}
			}
		}
		

		stage('Build FicsIt-Networks') {
			steps {
				bat label: 'Create project files', script: '.\\ue4\\Engine\\Binaries\\DotNET\\UnrealBuildTool.exe -projectfiles -project="%WORKSPACE%\\SatisfactoryModLoader\\FactoryGame.uproject" -game -rocket -progress'
				bat label: 'Build for Shipping', script: 'MSBuild.exe .\\SatisfactoryModLoader\\FactoryGame.sln /p:Configuration="Shipping" /p:Platform="Win64" /t:"Games\\FactoryGame"'
				bat label: 'Build for Editor', script: 'MSBuild.exe .\\SatisfactoryModLoader\\FactoryGame.sln /p:Configuration="Development Editor" /p:Platform="Win64" /t:"Games\\FactoryGame"'
			}
		}

		stage('Package FicsIt-Networks') {
			steps {
				retry(3) {
					bat label: 'Alpakit!', script: '.\\ue4\\Engine\\Build\\BatchFiles\\RunUAT.bat -ScriptsForProject="%WORKSPACE%\\SatisfactoryModLoader\\FactoryGame.uproject" PackagePlugin -Project="%WORKSPACE%\\SatisfactoryModLoader\\FactoryGame.uproject" -PluginName="%MOD_NAME%"'
				}
			}
		}

		stage('Archive') {
			when {
				not {
					changeRequest()
				}
			}
			
			steps {
				bat script: 'rename .\\SatisfactoryModLoader\\Saved\\ArchivedPlugins\\WindowsNoEditor\\${MOD_NAME}.zip ${MOD_NAME}_${BRANCH_NAME}_${BUILD_NUMBER}.zip'
				archiveArtifacts artifacts: 'SatisfactoryModLoader\\Saved\\ArchivedPlugins\\WindowsNoEditor\\${MOD_NAME}_${BRANCH_NAME}_${BUILD_NUMBER}.zip', fingerprint: true, onlyIfSuccessful: true
			}
		}
	}
	
	post {
		always {
			cleanWs()
			withCredentials([string(credentialsId: 'FINDiscordToken', variable: 'WEBHOOKURL')]) {
				discordSend description: "FIN Build", link: env.BUILD_URL, result: currentBuild.currentResult, title: JOB_NAME, webhookURL: "$WEBHOOKURL"
			}
		}
	}
}
