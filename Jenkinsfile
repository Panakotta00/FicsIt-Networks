pipeline {
	agent {
		label 'Windows2019'
	}

	options {
		disableConcurrentBuilds()
		skipDefaultCheckout(true)
	}

	stages {
		stage('SML') {
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

		stage('Checkout') {
			steps {
				dir("SatisfactoryModLoader/Plugins") {
					checkout scm: [
						$class: 'GitSCM',
						branches: scm.branches,
						extensions: [[
							$class: 'RelativeTargetDirectory',
							relativeTargetDir: 'FicsItNetworks'
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
					bat label: 'Apply SML Patch', script: 'git apply Plugins\\FicsItNetworks\\SML_Patch.patch'
					bat label: 'Apply Asset Patch', script: 'git apply %ASSETS%'
					bat label: 'Add WWise', script: '7z x %WWISE_PLUGIN% -oPlugins\\'
				}
			}
		}

		stage('Setup UE4') {
			steps {
				dir('ue4') {
					withCredentials([string(credentialsId: 'GitHub-API', variable: 'GITHUB_TOKEN')]) {
						retry(3) {
							bat label: '', script: 'github-release download --user SatisfactoryModdingUE --repo UnrealEngine -l -n "UnrealEngine-CSS-Editor-Win64.zip" > UnrealEngine-CSS-Editor-Win64.zip'
						}
						bat label: '', script: '7z x UnrealEngine-CSS-Editor-Win64.zip'
					}
					bat label: '', script: 'SetupScripts\\Register.bat'
				}
			}
		}
		

		stage('Build FicsIt-Networks') {
			steps {
				bat label: '', script: '.\\ue4\\Engine\\Binaries\\DotNET\\UnrealBuildTool.exe  -projectfiles -project="%WORKSPACE%\\FicsIt-Networks\\FactoryGame.uproject" -game -rocket -progress'
				bat label: '', script: 'MSBuild.exe .\\SatisfactoryModLoader\\FactoryGame.sln /p:Configuration="Shipping" /p:Platform="Win64" /t:"Games\\FactoryGame"'
				bat label: '', script: 'MSBuild.exe .\\SatisfactoryModLoader\\FactoryGame.sln /p:Configuration="Development Editor" /p:Platform="Win64" /t:"Games\\FactoryGame"'
			}
		}

		stage('Package FicsIt-Networks') {
			retry(3) {
				bat label: '', script: '.\\ue4\\Engine\\Build\\BatchFiles\\RunUAT.bat -ScriptsForProject="%WORKSPACE%\\FicsIt-Networks\\FactoryGame.uproject" PackagePlugin -Project="%WORKSPACE%\\FicsIt-Networks\\FactoryGame.uproject" -PluginName="FicsItNetworks"'
			}

			when {
				not {
					changeRequest()
				}
			}
			steps {
				archiveArtifacts artifacts: 'SatisfactoryModLoader\\Saved\\ArchivedPlugins\\WindowsNoEditor\\FicsItNetworks.zip', fingerprint: true, onlyIfSuccessful: true
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
