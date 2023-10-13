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
	                    name: "auto-header-update"
	                ]],
	                extensions: [[
	                    $class: 'RelativeTargetDirectory',
	                    relativeTargetDir: 'SatisfactoryModLoader',
                   	],[
	                    $class: 'CloneOption',
	                    timeout: 20,
	                ],[
	                    $class: 'CheckoutOption',
	                    timeout: 20,
	                ]],
	                userRemoteConfigs: [[
	                    url: 'https://github.com/satisfactorymodding/SatisfactoryModLoader.git'
	                ]]
	            ]
	        }
		}

		stage('Checkout') {
			steps {
				dir("SatisfactoryModLoader/Mods") {
					checkout scm: [
						$class: 'GitSCM',
						branches: scm.branches,
						extensions: [[
							$class: 'RelativeTargetDirectory',
							relativeTargetDir: "${MOD_NAME}"
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
					bat label: 'Apply Source Patch', script: 'git apply Mods\\%MOD_NAME%\\SML_Patch.patch -v'
					//bat label: 'Apply Asset Patch', script: 'git apply %ASSETS% -v'
					bat label: 'Add WWise', script: '7z x %WWISE_PLUGIN% -oPlugins\\'
				}
			}
		}

		stage('Setup UE4') {
			steps {
				dir('ue4') {
					/*withCredentials([string(credentialsId: 'SMR', variable: 'SMR_TOKEN')]) {
						retry(5) {
							bat label: 'Download UE', script: 'aria2c -x 8 -s 8 -c https://%SMR_TOKEN%@ci.ficsit.app/job/UE-4.25.3-CSS/lastSuccessfulBuild/artifact/UnrealEngine-CSS-Editor-Win64.zip'
						}
					}
					bat label: 'Copy UE', script: 'copy C:\\Jenkins\\UnrealEngine-CSS-Editor-Win64.zip .'
					bat label: 'Extract UE', script: '7z x UnrealEngine-CSS-Editor-Win64.zip'
					bat label: 'Register UE', script: 'SetupScripts\\Register.bat'*/
					withCredentials([string(credentialsId: 'GitHub-API', variable: 'GITHUB_TOKEN')]) {
                        retry(3) {
                            bat label: 'Download UE - Part 1', script: 'github-release download --user SatisfactoryModding --repo UnrealEngine -l -n "UnrealEngine-CSS-Editor-Win64.7z.001" > UnrealEngine-CSS-Editor-Win64.7z.001'
                            bat label: 'Download UE - Part 2', script: 'github-release download --user SatisfactoryModding --repo UnrealEngine -l -n "UnrealEngine-CSS-Editor-Win64.7z.002" > UnrealEngine-CSS-Editor-Win64.7z.002'
                            bat label: 'Download UE - Part 2', script: 'github-release download --user SatisfactoryModding --repo UnrealEngine -l -n "UnrealEngine-CSS-Editor-Win64.7z.003" > UnrealEngine-CSS-Editor-Win64.7z.003'
                        }
                        bat label: '', script: '7z x -mmt=10 UnrealEngine-CSS-Editor-Win64.7z.001'
                    }
                    bat label: '', script: 'SetupScripts\\Register.bat'
				}
			}
		}
		

		stage('Build FicsIt-Networks') {
			steps {
				bat label: 'Create project files', script: '.\\ue4\\Engine\\Binaries\\DotNET\\UnrealBuildTool\\UnrealBuildTool.exe -projectfiles -project="%WORKSPACE%\\SatisfactoryModLoader\\FactoryGame.uproject" -game -rocket -progress'
				bat label: 'Build for Shipping', script: 'MSBuild.exe /p:CL_MPCount=5 .\\SatisfactoryModLoader\\FactoryGame.sln /p:Configuration="Shipping" /p:Platform="Win64" /t:"Games\\FactoryGame"'
				bat label: 'Build for Editor', script: 'MSBuild.exe /p:CL_MPCount=5 .\\SatisfactoryModLoader\\FactoryGame.sln /p:Configuration="Development Editor" /p:Platform="Win64" /t:"Games\\FactoryGame"'
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
				bat script: "rename .\\SatisfactoryModLoader\\Saved\\ArchivedPlugins\\Windows\\${MOD_NAME}.zip ${MOD_NAME}_${BRANCH_NAME}_${BUILD_NUMBER}.zip"
				archiveArtifacts artifacts: "SatisfactoryModLoader\\Saved\\ArchivedPlugins\\Windows\\${MOD_NAME}_${BRANCH_NAME}_${BUILD_NUMBER}.zip", fingerprint: true, onlyIfSuccessful: true
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
